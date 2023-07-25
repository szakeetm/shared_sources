/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/


#include <thread>
#ifdef _WIN32
#include <process.h>
#elif not(defined(__MACOSX__) || defined(__APPLE__))
#include <cstring> //memset on unix
#endif

#include "SimulationManager.h"
//#include "Buffer_shared.h" // TODO: Move SHCONTROL to seperate file or SMP.h
#include "SMP.h"
#include "ProcessControl.h"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <cereal/archives/binary.hpp>
#ifdef MOLFLOW
#include "../src/Simulation/MolflowSimulation.h"
#endif

#ifdef SYNRAD
#include "../src/Simulation/SynradSimulation.h"
#endif
#include <Helper/ConsoleLogger.h>

SimulationManager::SimulationManager(int pid) {
    if(pid > -1)
        mainProcId = pid;
    else {
#ifdef _WIN32
        mainProcId = _getpid();
#else
        mainProcId = ::getpid();
#endif
    }
}

SimulationManager::~SimulationManager() {
    KillAllSimUnits();
}

//! Refresh proc status by looking for those that can be safely killed and remove them
int SimulationManager::refreshProcStatus() {
    int nbDead = 0;
    for(auto proc = simThreads.begin(); proc != simThreads.end() ; ){
        if(!(*proc).joinable()){
            auto myHandle = (*proc).native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
            TerminateThread(myHandle, 1);
#else
            //Linux
            pthread_cancel(myHandle);
#endif
            proc = this->simThreads.erase(proc);
            ++nbDead;
        }
        else{
            ++proc;
        }
    }
    return nbDead;
}

/*!
 * @brief Starts the simulation on all available simulation units
 * @return 0=start successful, 1=PROCESS_DONE state entered
 */
int SimulationManager::StartSimulation() {

    if (simulationChanged) {
        LoadSimulation(); //sets simulationChanged to false
    }

    if(asyncMode) {
        refreshProcStatus();
        if (simThreads.empty())
            throw std::logic_error("No active simulation threads.");

        if (ExecuteAndWait(COMMAND_RUN, PROCESS_RUN, 0, 0)) {
            throw std::runtime_error(MakeSubProcError("Subprocesses could not start the simulation"));
        }
    }
    else {
        procInformation.masterCmd  = COMMAND_RUN; // TODO: currently needed to not break the loop
        simController->Start();
    }

    if(allProcsDone){
        isRunning = false;
        return 1;
    }
    isRunning = true;
    return 0;
}

//! Call simulation controllers to stop running simulations
//! //interactive mode
int SimulationManager::StopSimulation() {
    isRunning = false;
    if(asyncMode) {
        refreshProcStatus();
        if (simThreads.empty()) {
            throw std::logic_error("No active simulation threads.");
        }

        if (ExecuteAndWait(COMMAND_PAUSE, PROCESS_READY, 0, 0))
            throw std::runtime_error(MakeSubProcError("Subprocesses could not stop the simulation"));
    }
    else {
        return 1;
    }

    return 0;
}

int SimulationManager::LoadSimulation(){
    if(asyncMode) {
        if (simThreads.empty())
            throw std::logic_error("No active simulation threads");

        if (ExecuteAndWait(COMMAND_LOAD, PROCESS_READY, 0, 0)) {
            std::string errString = "Failed to send geometry to sub process:\n";
            errString.append(GetErrorDetails());
            throw std::runtime_error(errString);
        }
        simulationChanged = false;
    }
    else{
        if(simController->Load()){
            std::cerr << "Failed to load simulation!" << std::endl;
            return 1;
        }
        simulationChanged = false;
    }

    return 0;
}

//! Create simulation handles for CPU simulations with n Threads
int SimulationManager::CreateCPUHandle() {
    uint32_t processId = mainProcId;

    //Get number of cores
    if(nbThreads == 0) { //Default: Use maximum available threads
#if defined(DEBUG)
        nbThreads = 1;
#else
#ifdef _WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        nbThreads = (size_t) sysinfo.dwNumberOfProcessors;
#else
        nbThreads = (unsigned int) sysconf(_SC_NPROCESSORS_ONLN);
#endif // WIN
#endif // DEBUG
    }

    size_t nbSimulations = 1; // nbThreads
    try{
        procInformation.Resize(nbThreads);
        simThreads.clear();
    }
    catch (const std::exception &e){
        std::cerr << "[SimManager] Invalid resize/clear " << nbThreads<< std::endl;
        throw std::runtime_error(e.what());
    }

 //Dirty
#ifdef MOLFLOW
    simulation = std::make_unique<MolflowSimulation>();
#endif
#ifdef SYNRAD
    simulation = std::make_unique<SynradSimulation>();
#endif
    simController = std::make_unique<SimulationController>((size_t)processId, (size_t)0, nbThreads, simulation.get(), &procInformation);
    
    if(asyncMode) {
        simThreads.emplace_back(
                std::thread(&SimulationController::controlledLoop, simController.get()));
        auto myHandle = simThreads.back().native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
        SetThreadPriority(myHandle, THREAD_PRIORITY_IDLE);
#else
        int policy;
        struct sched_param param{};
        pthread_getschedparam(myHandle, &policy, &param);
        param.sched_priority = sched_get_priority_min(policy);
        pthread_setschedparam(myHandle, policy, &param);
        //Check! Some documentation says it's always 0
#endif
    }

    return 0;
}

// return 1=error
int SimulationManager::CreateGPUHandle() {
    //not implemented
    return 1;
}

// return 1=error
int SimulationManager::CreateRemoteHandle() {
    //not implemented
    return 1;
}

/*!
 * @brief Creates Simulation Units and waits for their ready status
 * @return 0=all SimUnits are ready, else = ret Units are active, but not all could be launched
 */
int SimulationManager::InitSimulations() {
    CreateCPUHandle();
    return WaitForProcStatus(PROCESS_READY);
}

/*!
 * @brief Creates Simulation Units and waits for their ready status
 * @return 0=all SimUnits are ready, else = ret Units are active, but not all could be launched
 */
void SimulationManager::InitSimulation(std::shared_ptr<SimulationModel> model, GlobalSimuState *globStatePtr) {
    
    std::lock_guard<std::mutex> lock(model->modelMutex); //throws error if unsuccessful

    // Prepare simulation unit
    ResetSimulations();
    ForwardSimModel(model);
    ForwardGlobalCounter(globStatePtr, nullptr);
}

/*!
 * @brief Wait until all SimulationUnits are in successStatus or reach another endstate (error, done)
 * @param successStatus Process Status that should be waited for
 * @return 0 if wait is successful
 */
int SimulationManager::WaitForProcStatus(const uint8_t successStatus) {
    // Wait for completion
    bool finished = false;
    const int waitAmount = 250;
    int prevIncTime = 0; // save last time a wait increment has been set; allows for a dynamic/reasonable increase
    int waitTime = 0;
    int timeOutAt = 10000; // 10 sec; max time for an idle operation to timeout
    allProcsDone = true;
    hasErrorStatus = false;

    // struct, because vector of char arrays is forbidden w/ clang
    std::vector<std::string> prevSlaveStatuses(procInformation.subProcInfos.size());
    std::vector<std::string> newSlaveStatuses(procInformation.subProcInfos.size());

    for (size_t i = 0; i < procInformation.subProcInfos.size(); i++) {
        prevSlaveStatuses[i]=procInformation.subProcInfos[i].slaveStatus;
    }

	do {

		finished = true;

		for (size_t i = 0; i < procInformation.subProcInfos.size(); i++) {
			auto procState = procInformation.subProcInfos[i].slaveState;
			if (successStatus == PROCESS_KILLED) {// explicitly ask for killed state
				finished = finished && (procState == PROCESS_KILLED);
			}
			else {
				finished = finished && (procState == successStatus || procState == PROCESS_ERROR || procState == PROCESS_DONE);
			}
			if (procState == PROCESS_ERROR) {
				hasErrorStatus = true;
			}
			else if (procState == PROCESS_STARTING) {
				newSlaveStatuses[i] = procInformation.subProcInfos[i].slaveStatus;
				if (prevSlaveStatuses[i] != newSlaveStatuses[i]) { // if strings are different
					timeOutAt += (waitTime + 10000 < timeOutAt) ? (waitTime - prevIncTime) : (timeOutAt - waitTime +
						10000); // if task properly started, increase allowed wait time
					prevIncTime = waitTime;
				}
			}
			allProcsDone = allProcsDone && (procState == PROCESS_DONE);
		}

		if (!finished) {
			ProcessSleep(waitAmount);
			waitTime += waitAmount;
		}
	} while (!finished && waitTime < timeOutAt);

	return waitTime >= timeOutAt || hasErrorStatus; // 0 = finished, 1 = timeout
}

//! Forward a command to simulation controllers
void SimulationManager::ForwardCommand(const int command, const size_t param, const size_t param2) {

    procInformation.masterCmd = command;
    procInformation.cmdParam = param;
    procInformation.cmdParam2 = param2;
    for(auto & spi : procInformation.subProcInfos) {
        auto& procState = spi.slaveState;
        if(procState == PROCESS_READY || procState == PROCESS_RUN || procState==PROCESS_ERROR || procState==PROCESS_DONE) { // check if it'' ready before sending a new command
            procState = PROCESS_STARTING;
        }
    }
}

/*!
 * @brief Shortcut function combining ForwardCommand() and WaitForProcStatus() into a single call
 * @param command execution command for every subprocess
 * @param successStatus status that every subprocess has to reach
 * @param param additional command parameter
 * @return 0=success, 1=fail
 */
int SimulationManager::ExecuteAndWait(const int command, const uint8_t successStatus, const size_t param,
                                      const size_t param2) {
    ForwardCommand(command, param, param2); // sets master state to command, param and param2, and sets processes to "starting" if they are ready
    if (!WaitForProcStatus(successStatus)) { // and wait
        return 0;
    }
    return 1;
}

int SimulationManager::KillAllSimUnits() {
    if( !simThreads.empty() ) {
        if(ExecuteAndWait(COMMAND_EXIT, PROCESS_KILLED)){ // execute
            // Force kill
            simController->EmergencyExit();
            if(ExecuteAndWait(COMMAND_EXIT, PROCESS_KILLED)) {
                int i = 0;
                for (auto tIter = simThreads.begin(); tIter != simThreads.end(); ++i) {
                    if (procInformation.subProcInfos[i].slaveState != PROCESS_KILLED) {
                        auto nativeHandle = simThreads[i].native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
                        //Windows
                        TerminateThread(nativeHandle, 1);
#else
                        //Linux
                        int s;
                        s = pthread_cancel(nativeHandle);
                        if (s != 0)
                            Log::console_msg(1, "pthread_cancel: {}\n", s);
                        tIter->detach();
#endif
                        //assume that the process doesn't exist, so remove it from our management structure
                        try {
                            tIter = simThreads.erase(tIter);
                        }
                        catch (const std::exception &e) {
                            throw std::runtime_error(fmt::format("Could not terminate subprocesses: {}\n",e.what())); // proc couldn't be killed!?
                        }
                        catch (...) {
                            throw std::runtime_error("Could not terminate subprocesses.\n"); // proc couldn't be killed!?
                        }
                    }
                }
            }
        }

        for(size_t i=0;i<simThreads.size();i++) {
            if (procInformation.subProcInfos[i].slaveState == PROCESS_KILLED) {
                simThreads[i].join();
            }
            else{
                if(ExecuteAndWait(COMMAND_EXIT, PROCESS_KILLED))
                    exit(1);
/*                auto nativeHandle = simThreads[i].first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
                //Windows
                TerminateThread(nativeHandle, 1);
#else
                //Linux
                pthread_cancel(nativeHandle);
#endif*/
            }
        }
        simThreads.clear();
    }
    nbThreads = 0;
    return 0;
}

int SimulationManager::ResetSimulations() {
    if(asyncMode) {
        if (ExecuteAndWait(COMMAND_CLOSE, PROCESS_READY, 0, 0))
            throw std::runtime_error(MakeSubProcError("Subprocesses could not restart"));
    }
    else {
        simController->Reset();
    }
    
    return 0;
}

int SimulationManager::ResetHits() {
    isRunning = false;
    if(asyncMode) {
        if (ExecuteAndWait(COMMAND_RESET, PROCESS_READY, 0, 0))
            throw std::runtime_error(MakeSubProcError("Subprocesses could not reset hits"));
    }
    else {
        simController->Reset();
    }
    return 0;
}

int SimulationManager::GetProcStatus(ProcComm &procInfoList) {
    if(simThreads.empty())
        return 1;

    procInfoList.subProcInfos = procInformation.subProcInfos;

    return 0;
}

int SimulationManager::GetProcStatus(size_t *states, std::vector<std::string>& statusStrings) {

    if(statusStrings.size() < procInformation.subProcInfos.size())
        return 1;

    for (size_t i = 0; i < procInformation.subProcInfos.size(); i++) {
        //states[i] = shMaster->procInformation[i].masterCmd;
        states[i] = procInformation.subProcInfos[i].slaveState;
        statusStrings[i] = procInformation.subProcInfos[i].slaveStatus;
    }
    return 0;
}

/*!
 * @brief Actively check running state (evaluate done & error)
 * @return 1 if simulation processes are running, 0 else
 */
bool SimulationManager::GetRunningStatus(){

    bool done = true;
    for (auto & i : procInformation.subProcInfos) {
        auto procState = i.slaveState;
        done = done & (procState==PROCESS_ERROR || procState==PROCESS_DONE);
        if( procState==PROCESS_ERROR ) {
            hasErrorStatus = true;
        }
    }

    allProcsDone = done;
    isRunning = isRunning && !allProcsDone;

    return isRunning;
}

/*!
 * @brief Return error information or current running state in case of a hangup
 * @return char array containing proc status (and error message/s)
 */
std::string SimulationManager::GetErrorDetails() {

    ProcComm procInfoPtr;
    GetProcStatus(procInfoPtr);

    std::string err;

    for (size_t i = 0; i < procInfoPtr.subProcInfos.size(); i++) {
        size_t state = procInfoPtr.subProcInfos[i].slaveState;
        if (state == PROCESS_ERROR) {
            err.append(fmt::format("[Thread #{}] {}: {}", i, prStates[state], procInfoPtr.subProcInfos[i].slaveStatus));
        } else {
            err.append(fmt::format("[Thread #{}] {}", i, prStates[state]));
        }
    }
    return err;
}

std::string SimulationManager::MakeSubProcError(const std::string& message) {
    std::string errString;
    if (message.empty()){
        errString.append("Bad response from sub process(es):\n");
        errString.append(GetErrorDetails());
    }
    else{
        errString.append(message);
        errString.append(":\n");
        errString.append(GetErrorDetails());
    }
    return errString;
}


int SimulationManager::ShareWithSimUnits(void *data, size_t size, LoadType loadType) {
    if(loadType < LoadType::NLOADERTYPES){
        /*if(CreateLoaderDP(size))
            return 1;
        if(UploadToLoader(data, size))
            return 1;*/
    }


    switch (loadType) {
        case LoadType::LOADGEOM:{
            if (ExecuteAndWait(COMMAND_LOAD, PROCESS_READY, size, 0)) {
                //CloseLoaderDP();
                std::string errString = "Failed to send geometry to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            //CloseLoaderDP();
            break;
        }
        case LoadType::LOADPARAM:{

            //if (ExecuteAndWait(COMMAND_UPDATEPARAMS, isRunning ? PROCESS_RUN : PROCESS_READY, size, isRunning ? PROCESS_RUN : PROCESS_READY)) {
            if (ExecuteAndWait(COMMAND_UPDATEPARAMS, PROCESS_READY, size, isRunning ? PROCESS_RUN : PROCESS_READY)) {
                //CloseLoaderDP();
                std::string errString = "Failed to send params to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            //CloseLoaderDP();
            if(isRunning) { // restart
                StartSimulation();
            }
            break;
        }
        case LoadType::LOADHITS:{
        }
        default:{
            // Unspecified load type
            return 1;
        }
    }

    return 0;
}

void SimulationManager::ForwardGlobalCounter(GlobalSimuState *simStatePtr, ParticleLog *particleLogPtr) {
        auto lock = GetHitLock(simStatePtr, 10000);
        if(!lock) return;
        simulation->globStatePtr = simStatePtr;
        simulation->globParticleLogPtr = particleLogPtr;
}

void SimulationManager::ForwardSimModel(std::shared_ptr<SimulationModel> model) { //also shares ownership
    simulation->model = model;
}

// Create hard copy for local usage and resie particle logger
void SimulationManager::ForwardOtfParams(OntheflySimulationParams *otfParams) {
    simulation->model->otfParams = *otfParams;
    simulation->ReinitializeParticleLog();
}

/**
* \brief Saves current facet hit counter from cache to results, only for constant flow (moment 0)
* Sufficient for .geo and .txt formats, for .xml moment results are written during the loading
*/
void SimulationManager::ForwardFacetHitCounts(std::vector<FacetHitBuffer*>& hitCaches) {
        if(simulation->globStatePtr->facetStates.size() != hitCaches.size()) return;
        auto lock = GetHitLock(simulation->globStatePtr,10000);
        if(!lock) return;
        for (size_t i = 0; i < hitCaches.size(); i++) {
            simulation->globStatePtr->facetStates[i].momentResults[0].hits = *hitCaches[i];
        }
}

/*
int SimulationManager::IncreasePriority() {
#if defined(_WIN32) && defined(_MSC_VER)
    // https://cpp.hotexamples.com/de/examples/-/-/SetPriorityClass/cpp-setpriorityclass-function-examples.html
    HANDLE hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS);
#else

#endif

    for(auto& handle : simThreads) {
        auto myHandle = handle.first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
        SetThreadPriority(myHandle, THREAD_PRIORITY_HIGHEST);
#else
        int policy;
        struct sched_param param{};
        pthread_getschedparam(myHandle, &policy, &param);
        param.sched_priority = sched_get_priority_min(policy);
        pthread_setschedparam(myHandle, policy, &param);
        //Check! Some documentation says it's always 0
#endif
    }

    return 0;
}

int SimulationManager::DecreasePriority() {
#if defined(_WIN32) && defined(_MSC_VER)
    HANDLE hProcess = GetCurrentProcess();
    SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS);
#else

#endif
    for(auto& handle : simThreads) {
        auto myHandle = handle.first.native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
        SetThreadPriority(myHandle, THREAD_PRIORITY_IDLE);
#else
        int policy;
            struct sched_param param{};
            pthread_getschedparam(myHandle, &policy, &param);
            param.sched_priority = sched_get_priority_min(policy);
            pthread_setschedparam(myHandle, policy, &param);
            //Check! Some documentation says it's always 0
#endif
    }
    return 0;
}

int SimulationManager::RefreshRNGSeed(bool fixed) {
    if(simulations.empty() || nbThreads == 0)
        return 1;
    for(auto& sim : simulations){
        sim->ConstructParticleTracers(nbThreads, fixed);
    }

    return 0;
}
*/