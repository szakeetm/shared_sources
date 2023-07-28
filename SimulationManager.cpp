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
    KillSimulation();
}

//! Refresh proc status by looking for those that can be safely killed and remove them
int SimulationManager::refreshProcStatus() {
    if (!controllerLoopThread) return 1; //invalid
    if (!controllerLoopThread->joinable()) {
        auto myHandle = controllerLoopThread->native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
        TerminateThread(myHandle, 1);
#else
        //Linux
        pthread_cancel(myHandle);
#endif
        controllerLoopThread.reset();
        return 1; //invalid thread
    }
    else return 0; //valid thread
}

/*!
 * @brief Starts the simulation on all available simulation units
 * @return 0=start successful, 1=PROCESS_DONE state entered
 */
int SimulationManager::StartSimulation(LoadStatus_abstract* loadStatus) {

    if (simulationChanged) {
        LoadSimulation(loadStatus); //sets simulationChanged to false
    }

    if(asyncMode) {
        refreshProcStatus();
        if (!controllerLoopThread)
            throw std::logic_error("No active simulation thread.");

        if (ExecuteAndWait(SimCommand::Run, SimState::Running, 0, 0,loadStatus)) {
            throw std::runtime_error(MakeSubProcError("Subprocesses could not start the simulation"));
        }
    }
    else {
        procInformation.masterCmd  = SimCommand::Run; // TODO: currently needed to not break the loop
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
int SimulationManager::StopSimulation(LoadStatus_abstract* loadStatus) {
    isRunning = false;
    if(asyncMode) {
        refreshProcStatus();
        if (!controllerLoopThread) {
            throw std::logic_error("No active simulation thread.");
        }

        if (ExecuteAndWait(SimCommand::Pause, SimState::Ready, 0, 0, loadStatus))
            throw std::runtime_error(MakeSubProcError("Subprocesses could not stop the simulation"));
    }
    else {
        return 1;
    }

    return 0;
}

int SimulationManager::LoadSimulation(LoadStatus_abstract* loadStatus){
    if(asyncMode) {
        if (!controllerLoopThread)
            throw std::logic_error("No active simulation thread");

        if (ExecuteAndWait(SimCommand::Load, SimState::Ready, 0, 0,loadStatus)) {
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
int SimulationManager::CreateCPUHandle(LoadStatus_abstract* loadStatus) {
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

    try{
        procInformation.Resize(nbThreads);
        procInformation.UpdateMasterStatus("Deleting old simulation...", loadStatus);
        controllerLoopThread.reset();
    }
    catch (const std::exception &e){
        std::cerr << "[SimManager] Invalid resize/clear " << nbThreads<< std::endl;
        throw std::runtime_error(e.what());
    }

    procInformation.UpdateMasterStatus("Creating new simulation...", loadStatus);
 //Dirty
#ifdef MOLFLOW
    simulation = std::make_unique<MolflowSimulation>();
#endif
#ifdef SYNRAD
    simulation = std::make_unique<SynradSimulation>();
#endif
    procInformation.UpdateMasterStatus("Creating new simulation controller...", loadStatus);
    simController = std::make_unique<SimulationController>((size_t)processId, (size_t)0, nbThreads, simulation.get(), procInformation);
    
    if(asyncMode) {
        controllerLoopThread = std::make_unique<std::thread>(&SimulationController::controlledLoop, simController.get());
        auto myHandle = controllerLoopThread->native_handle();
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

/*!
 * @brief Creates Simulation Units and waits for their ready status
 * @return 0=all SimUnits are ready, else = ret Units are active, but not all could be launched
 */
int SimulationManager::SetUpSimulation(LoadStatus_abstract* loadStatus) {
    procInformation.UpdateMasterStatus("Creating worker threads...", loadStatus);
    CreateCPUHandle();
    procInformation.UpdateMasterStatus("Waiting for worker threads to be ready...", loadStatus);
    auto result = WaitForProcStatus(SimState::Ready);
    procInformation.UpdateMasterStatus("", loadStatus);
    return result;
}

/*!
 * @brief Creates Simulation Units and waits for their ready status
 * @return 0=all SimUnits are ready, else = ret Units are active, but not all could be launched
 */
//Called from CLI and test suite
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
int SimulationManager::WaitForProcStatus(const SimState successState, LoadStatus_abstract* loadStatus) {
    // Wait for completion
    bool finished = false;
    const int waitAmount_ms = 500;
    allProcsDone = true;
    hasErrorStatus = false;
    bool abortRequested = false;
	do {

		finished = true;

		for (size_t i = 0; i < procInformation.threadInfos.size(); i++) {
			auto procState = procInformation.threadInfos[i].slaveState;
			if (successState == SimState::Killed) {// explicitly ask for killed state
				finished = finished && (procState == SimState::Killed);
			}
			else {
				finished = finished && (procState == successState || procState == SimState::InError || procState == SimState::Ready);
			}
			if (procState == SimState::InError) {
				hasErrorStatus = true;
			}
			allProcsDone = allProcsDone && (procState == SimState::Ready);
		}
        //if (loadStatus) finished = false; //debug
		if (!finished) {
            if (loadStatus) {
                loadStatus->procStateCache.procDataMutex.lock();
                loadStatus->procStateCache = procInformation;
                loadStatus->procStateCache.procDataMutex.unlock();
                loadStatus->Update();
            }

			ProcessSleep(waitAmount_ms);

            if (loadStatus) {
                abortRequested = loadStatus->abortRequested;
            }
		}
    } while (!finished || abortRequested);

	return (hasErrorStatus || abortRequested); // 0 = finished, 1 = error or aborted
}

//! Forward a command to simulation controllers
void SimulationManager::ForwardCommand(const SimCommand command, const size_t param, const size_t param2) {

    procInformation.masterCmd = command;
    procInformation.cmdParam = param;
    procInformation.cmdParam2 = param2;
    
    for(auto & spi : procInformation.threadInfos) {
        auto& procState = spi.slaveState;
        //if(procState == SimState::Ready || procState == SimState::Running || procState==SimState::InError || procState==SimState::Finished) { // check if ready before sending a new command
            procState = SimState::ExecutingCommand; //Generic, since not every command (ex. pause, reset, load) has its dedicated SimState
        //}
    }
}

/*!
 * @brief Shortcut function combining ForwardCommand() and WaitForProcStatus() into a single call
 * @param command execution command for every subprocess
 * @param successStatus status that every subprocess has to reach
 * @param param additional command parameter
 * @return 0=success, 1=fail
 */
int SimulationManager::ExecuteAndWait(const SimCommand command, const SimState successState,
    const size_t param, const size_t param2, LoadStatus_abstract* loadStatus) {
    ForwardCommand(command, param, param2); // sets master state to command, param and param2, and sets processes to "starting" if they are ready
    if (!WaitForProcStatus(successState, loadStatus)) { // and wait
        return 0;
    }
    return 1;
}

int SimulationManager::KillSimulation(LoadStatus_abstract* loadStatus) {
    if( controllerLoopThread ) {
        if(ExecuteAndWait(SimCommand::Kill, SimState::Killed,0,0,loadStatus)){ // execute
            // Force kill
            simController->EmergencyExit(); //Request particle tracers to not do more MC steps
            if(ExecuteAndWait(SimCommand::Kill, SimState::Killed,0,0,loadStatus)) { 
                int i = 0;
                if (procInformation.threadInfos[i].slaveState != SimState::Killed) {
                    auto nativeHandle = controllerLoopThread->native_handle();
#if defined(_WIN32) && defined(_MSC_VER)
                    //Windows
                    TerminateThread(nativeHandle, 1);
#else
                    //Linux
                    int s;
                    s = pthread_cancel(nativeHandle);
                    if (s != 0)
                        Log::console_msg(1, "pthread_cancel: {}\n", s);
                    controllerLoopThread->detach();
#endif
                    //assume that the process doesn't exist, so remove it from our management structure
                    try {
                        controllerLoopThread.reset();
                    }
                    catch (const std::exception &e) {
                        throw std::runtime_error(fmt::format("Could not terminate simulation thread: {}\n",e.what())); // proc couldn't be killed!?
                    }
                    catch (...) {
                        throw std::runtime_error("Could not terminate simulation thread.\n"); // proc couldn't be killed!?
                    }
                }
            }
        }
        bool allKilled = true;
        for (size_t i = 0; i < procInformation.threadInfos.size(); i++) {
            allKilled = allKilled && procInformation.threadInfos[i].slaveState == SimState::Killed;
        }
        if (allKilled) {
            controllerLoopThread->join();
        } else {
            if (ExecuteAndWait(SimCommand::Kill, SimState::Killed, 0, 0, loadStatus)) {
                exit(1);
            }
        }
        controllerLoopThread.reset();
    }
    nbThreads = 0;
    return 0;
}

int SimulationManager::ResetSimulations() {
    if(asyncMode) {
        if (ExecuteAndWait(SimCommand::Reset, SimState::Ready, 0, 0))
            throw std::runtime_error(MakeSubProcError("Subprocesses could not reset"));
    }
    else {
        simController->Reset();
    }
    
    return 0;
}

int SimulationManager::GetProcStatus(ProcComm &procInfoList) {
    if(!controllerLoopThread)
        return 1;

    procInfoList.threadInfos = procInformation.threadInfos;

    return 0;
}

std::string SimulationManager::GetMasterStatus()
{
    return fmt::format("[{}] {}", simCommandStrings[procInformation.masterCmd], procInformation.masterStatus);
}

int SimulationManager::GetProcStatus(size_t *states, std::vector<std::string>& statusStrings) {

    if(statusStrings.size() < procInformation.threadInfos.size())
        return 1;

    for (size_t i = 0; i < procInformation.threadInfos.size(); i++) {
        //states[i] = shMaster->procInformation[i].masterCmd;
        states[i] = procInformation.threadInfos[i].slaveState;
        statusStrings[i] = procInformation.threadInfos[i].slaveStatus;
    }
    return 0;
}

/*!
 * @brief Actively check running state (evaluate done & error)
 * @return 1 if simulation processes are running, 0 else
 */
bool SimulationManager::IsRunning(){

    bool done = true;
    for (auto & i : procInformation.threadInfos) {
        auto procState = i.slaveState;
        done = done && (procState==SimState::InError || procState==SimState::Ready);
        if( procState==SimState::InError ) {
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

    for (size_t i = 0; i < procInfoPtr.threadInfos.size(); i++) {
        auto state = procInfoPtr.threadInfos[i].slaveState;
        if (state == SimState::InError) {
            err.append(fmt::format("[Thread #{}] {}: {}", i, simStateStrings.at(state), procInfoPtr.threadInfos[i].slaveStatus));
        } else {
            err.append(fmt::format("[Thread #{}] {}", i, simStateStrings.at(state)));
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


int SimulationManager::ShareWithSimUnits(void *data, size_t size, LoadType loadType, LoadStatus_abstract* loadStatus) {
    if(loadType < LoadType::NLOADERTYPES){
        /*if(CreateLoaderDP(size))
            return 1;
        if(UploadToLoader(data, size))
            return 1;*/
    }


    switch (loadType) {
        case LoadType::LOADGEOM:{
            if (ExecuteAndWait(SimCommand::Load, SimState::Ready, size, 0, loadStatus)) {
                std::string errString = "Failed to send geometry to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            break;
        }
        case LoadType::LOADPARAM:{
            if (ExecuteAndWait(SimCommand::UpdateParams, SimState::Ready, size, isRunning ? SimState::Running : SimState::Ready, loadStatus)) {
                std::string errString = "Failed to send params to sub process:\n";
                errString.append(GetErrorDetails());
                throw std::runtime_error(errString);
            }
            if(isRunning) { // restart
                StartSimulation(loadStatus);
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

    for(auto& handle : simThread) {
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
    for(auto& handle : simThread) {
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