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

#include <cmath>
#include <sstream>
#include "SimulationController.h"
#include "../src/Simulation/Simulation.h"
#include "ProcessControl.h"
#include <Helper/ConsoleLogger.h>
#include <Helper/OutputHelper.h>

#include <omp.h>

#if not defined(_MSC_VER)
#include <sys/time.h>
#include <sys/resource.h>
#endif



#ifdef _WIN32
#include <process.h>
#endif

#define WAITTIME    500  // Answer in STOP mode


SimThread::SimThread(ProcComm *procInfo, Simulation_Abstract *sim, size_t threadNum) {
    this->threadNum = threadNum;
    masterProcInfo = procInfo;
    timeLimit = 0.0;
    simulation = sim;
    stepsPerSec = 1.0;
    particleTracer = nullptr;
    desLimitReachedOrDesError = false;
    localDesLimit = 0;
}


// todo: fuse with runSimulation1sec()
// Should allow simulation for N steps opposed to T seconds
int SimThread::advanceForSteps(size_t desorptions) {
    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : std::ceil(stepsPerSec + 0.5);

    double timeStart = omp_get_wtime();
    double timeEnd = timeStart;
    do {
        desLimitReachedOrDesError = particleTracer->SimulationMCStep(nbStep, threadNum, desorptions);
        timeEnd = omp_get_wtime();

        const double elapsedTimeMs = (timeEnd - timeStart); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs > 1e-6)
            stepsPerSec = (10.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run

        desorptions -= particleTracer->tmpState.globalStats.globalHits.nbDesorbed;
    } while (desorptions);


    return 0;
}

// run until end or until autosaveTime check
int SimThread::advanceForTime(double simDuration) {

    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : std::ceil(stepsPerSec + 0.5);

    double timeStart = omp_get_wtime();
    double timeEnd = timeStart;
    do {
        desLimitReachedOrDesError = particleTracer->SimulationMCStep(nbStep, threadNum, 0);
        timeEnd = omp_get_wtime();

        const double elapsedTimeMs = (timeEnd - timeStart); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs > 1e-6)
            stepsPerSec = (10.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run

    } while (simDuration > timeEnd - timeStart);


    return 0;
}

/**
* \brief Simulation loop for an individual thread
 * \return true when simulation end has been reached via desorption limit, false otherwise
 */

bool SimThread::runLoop() {
    bool finishLoop;
    bool lastUpdateOk = false;

    double timeStart = omp_get_wtime();
    double timeLoopStart = timeStart;
    double timeEnd;
    do {
        setMyStatus(ConstructThreadStatus());
        size_t desorptions = localDesLimit;
        desLimitReachedOrDesError = runSimulation1sec(desorptions); // Run for 1 sec
        
        timeEnd = omp_get_wtime();

        bool forceQueue = timeEnd-timeLoopStart > 60 || threadNum == 0; // update after 60s of no update or when thread 0 is called
        if (masterProcInfo->activeProcs.front() == threadNum || forceQueue) {
            size_t readdOnFail = 0;
            if(simulation->model->otfParams.desorptionLimit > 0){
                if(localDesLimit > particleTracer->tmpState.globalStats.globalHits.nbDesorbed) {
                    localDesLimit -= particleTracer->tmpState.globalStats.globalHits.nbDesorbed;
                    readdOnFail = particleTracer->tmpState.globalStats.globalHits.nbDesorbed;
                }
                else localDesLimit = 0;
            }

            size_t timeOut = lastUpdateOk ? 0 : 100; //ms
            lastUpdateOk = particleTracer->UpdateHitsAndLog(simulation->globState, simulation->globParticleLog,
                                                timeOut); // Update hit with 100ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).

            if(!lastUpdateOk) // if update failed, the desorption limit is invalid and has to be reverted
                localDesLimit += readdOnFail;

            if(masterProcInfo->activeProcs.front() == threadNum)
                masterProcInfo->NextSubProc();
            timeLoopStart = omp_get_wtime();
        }
        else{
            lastUpdateOk = false;
        }
        finishLoop = desLimitReachedOrDesError || (this->particleTracer->model->otfParams.timeLimit != 0 && ((timeEnd-timeStart) >= this->particleTracer->model->otfParams.timeLimit)) || (masterProcInfo->masterCmd != COMMAND_RUN) || (masterProcInfo->subProcInfos[threadNum].slaveState == PROCESS_ERROR);
    } while (!finishLoop);

    masterProcInfo->RemoveAsActive(threadNum);
    if (!lastUpdateOk) {
        setMyStatus("Final hit update...");
        particleTracer->UpdateHitsAndLog(simulation->globState, simulation->globParticleLog,
                             20000); // Update hit with 20s timeout
    }
    return desLimitReachedOrDesError;
}

void SimThread::setMyStatus(const std::string& msg) const { //Writes to master's procInfo
    masterProcInfo->subProcInfos[threadNum].slaveStatus=msg;
}

[[nodiscard]] std::string SimThread::ConstructThreadStatus() const {
    size_t count = particleTracer->totalDesorbed + particleTracer->tmpState.globalStats.globalHits.nbDesorbed;;

    size_t max = 0;
    if (simulation->model->otfParams.nbProcess)
        max = (simulation->model->otfParams.desorptionLimit / simulation->model->otfParams.nbProcess)
                + ((this->threadNum < simulation->model->otfParams.desorptionLimit % simulation->model->otfParams.nbProcess) ? 1 : 0);

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        return fmt::format("{}/{} des ({:.1f}%)",count, max, percent);
    } else {
        return fmt::format("{} des", count);
    }
}

/**
* \brief A "single (1sec)" MC step of a simulation run for a given thread
 * \return false when simulation continues, true when desorption limit is reached or des error
 */
bool SimThread::runSimulation1sec(const size_t desorptions) {
    // 1s step
    size_t nbStep = (stepsPerSec <= 0.0) ? 250.0 : std::ceil(stepsPerSec + 0.5);

    setMyStatus(fmt::format("{} [{} hits/s]",ConstructThreadStatus(), nbStep));

    // Check end of simulation
    bool limitReachedOrDesError = false;
    size_t remainingDes = 0;

    if (particleTracer->model->otfParams.desorptionLimit > 0) {
        if (desorptions <= particleTracer->tmpState.globalStats.globalHits.nbDesorbed){
            limitReachedOrDesError = true;
        }
        else {
            remainingDes = desorptions - particleTracer->tmpState.globalStats.globalHits.nbDesorbed;
        }
    }

    if(!limitReachedOrDesError) {
        double start_time = omp_get_wtime();
        limitReachedOrDesError = particleTracer->SimulationMCStep(nbStep, threadNum, remainingDes);
        double end_time = omp_get_wtime();


        if (limitReachedOrDesError) // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
        {
            const double elapsedTimeMs = (end_time - start_time);
            if (elapsedTimeMs != 0.0)
                stepsPerSec = static_cast<double>(nbStep) / elapsedTimeMs; // every 1.0 second
            else
                stepsPerSec = (100.0 * static_cast<double>(nbStep)); // in case of fast initial run
        }
    }
    return limitReachedOrDesError;
}

SimulationController::SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                                           Simulation_Abstract *simulationInstance, ProcComm *pInfo) {
    this->prIdx = procIdx;
    this->parentPID = parentPID;
    if (nbThreads == 0)
#if defined(DEBUG)
        this->nbThreads = 1;
#else
        this->nbThreads = omp_get_max_threads();
#endif
    else
        this->nbThreads = nbThreads;

    simulation = simulationInstance; // TODO: Find a nicer way to manager derived simulationunit for Molflow and Synrad
    procInfo = pInfo; // Set from outside to link with general structure

    loadOk = false;

    // resetControls()
    stepsPerSec = 0.0;
    endState = false;
    lastHitUpdateOK = true;

    SetRuntimeInfo();

    //DEBUG_PRINT("Controller created (%zd bytes)\n", sizeof(SHCONTROL));
    SetReady(false);
}

SimulationController::~SimulationController() = default;

SimulationController::SimulationController(SimulationController &&o) noexcept {
    simulation = o.simulation;
    procInfo = o.procInfo;

    o.simulation = nullptr;
    o.procInfo = nullptr;

    stepsPerSec = o.stepsPerSec;
    endState = o.endState;
    lastHitUpdateOK = o.lastHitUpdateOK;

    prIdx = o.prIdx;
    parentPID = o.parentPID;
    nbThreads = o.nbThreads;
    loadOk = o.loadOk;

    simThreads.reserve(nbThreads);
    for (size_t t = 0; t < nbThreads; t++) {
        simThreads.emplace_back(
                SimThread(procInfo, simulation, t));
        simThreads.back().particleTracer = simulation->GetParticleTracer(t);
    }
}

int SimulationController::resetControls() {
    lastHitUpdateOK = true;
    endState = false;

    stepsPerSec = 0.0;

    return 0;
}

int SimulationController::SetRuntimeInfo() {

    // Update runtime information
    for (auto &pInfo : procInfo->subProcInfos)
        GetProcInfo(pInfo.procId, &pInfo.runtimeInfo);

    return 0;
}

int SimulationController::ClearCommand() {

    procInfo->masterCmd = COMMAND_NONE;
    procInfo->cmdParam = 0;
    procInfo->cmdParam2 = 0;
    SetThreadStates(PROCESS_READY,GetThreadStatuses());

    return 0;
}

int SimulationController::SetThreadStates(size_t state, const std::string &status, bool changeState, bool changeStatus) {

    if (changeState) {
        DEBUG_PRINT("setstate %s\n", prStates[state]);
        //master->procInformation[prIdx].masterCmd = state;
        for (auto &pInfo : procInfo->subProcInfos) {
            pInfo.slaveState = state;
        }
    }
    if (changeStatus) {
        for (auto &pInfo : procInfo->subProcInfos) {
            pInfo.slaveStatus=status;
        }
    }

    if(state == PROCESS_ERROR){
        procInfo->masterCmd = PROCESS_WAIT;
    }
    return 0;
}

int SimulationController::SetThreadStates(size_t state, const std::vector<std::string> &status, bool changeState, bool changeStatus) {

    if (changeState) {
        DEBUG_PRINT("setstate %s\n", prStates[state]);
        for (auto &pInfo : procInfo->subProcInfos) {
            pInfo.slaveState = state;
        }
    }
    if (changeStatus) {
        if(procInfo->subProcInfos.size() != status.size()){
            for (auto &pInfo : procInfo->subProcInfos) {
               pInfo.slaveStatus="invalid state (subprocess number mismatch)";
            }
        }
        else {
            size_t pInd = 0;
            for (auto &pInfo : procInfo->subProcInfos) {
                pInfo.slaveStatus=status[pInd++];
            }
        }
    }

    if(state == PROCESS_ERROR){
        procInfo->masterCmd = PROCESS_WAIT;
    }
    return 0;
}

std::vector<std::string> SimulationController::GetThreadStatuses() {

    std::vector<std::string> threadStatuses(nbThreads);
    if (!simulation) {
        threadStatuses.assign(nbThreads, "[NONE]");
    }
    else{
        for(size_t threadId = 0; threadId < nbThreads; ++threadId) {
            auto* particleTracer = simulation->GetParticleTracer(threadId);
            if(particleTracer == nullptr) break;
            if(!particleTracer->tmpState.initialized){
                threadStatuses[threadId]= "[NONE]";
            }
            else {
                threadStatuses[threadId] = simThreads[threadId].ConstructThreadStatus();
            }
        }
    }
    return threadStatuses;
}

void SimulationController::SetErrorSub(const std::string& message) {
    Log::console_error("Error: {}\n", message);
    SetThreadStates(PROCESS_ERROR, message);
}

void SimulationController::SetStatus(const std::string& status) {
    for (auto &pInfo : procInfo->subProcInfos) {
        pInfo.slaveStatus=status;
    }
}

void SimulationController::SetReady(const bool loadOk) {

    if (loadOk)
        SetThreadStates(PROCESS_READY, this->GetThreadStatuses());
    else
        SetThreadStates(PROCESS_READY, "(No geometry)");

    ClearCommand();
}

// Returns the thread state if equal for all, otherwise PROCESS_ERROR
size_t SimulationController::GetThreadStates() const {
    size_t locState = procInfo->subProcInfos[0].slaveState;
    for (auto &pInfo : procInfo->subProcInfos) {
        if (locState != pInfo.slaveState)
            return PROCESS_ERROR;
    }
    return locState;
}

// Main loop
int SimulationController::controlledLoop(int argc, char **argv) {
    endState = false;
    loadOk = false;
    while (!endState) {
        bool eos = false;
        switch (procInfo->masterCmd) {

            case COMMAND_LOAD: {
                Load();
                break;
            }
            case COMMAND_UPDATEPARAMS: {
                SetThreadStates(PROCESS_WAIT, GetThreadStatuses());
                DEBUG_PRINT("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (UpdateParams()) {
                    SetReady(loadOk);
                    //SetThreadStates(procInfo->cmdParam2, GetThreadStatuses());
                } else {
                    SetThreadStates(PROCESS_ERROR, "Could not update parameters");
                }
                break;
            }
            case COMMAND_RUN: {
                Start();
                break;
            }
            case COMMAND_PAUSE: {
                DEBUG_PRINT("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetReady(loadOk);
                break;
            }
            case COMMAND_RESET: {
                Reset();
                break;
            }
            case COMMAND_EXIT: {
                DEBUG_PRINT("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                endState = true;
                break;
            }
            case COMMAND_CLOSE: {
                DEBUG_PRINT("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                auto* sim = simulation;
                //for (auto &sim : *simulation)
                    sim->ClearSimulation();
                loadOk = false;
                SetReady(loadOk);
                break;
            }
            default: {
                ProcessSleep(WAITTIME);
                break;
            }
        }
    }
    SetThreadStates(PROCESS_KILLED, "Process terminated peacefully");
    return 0;
}

/**
* \brief Call a rebuild of the ADS
 * \return 0> error code, 0 when ok
 */
int SimulationController::RebuildAccel() {
    if (simulation->RebuildAccelStructure()) {
        return 1;
    }
    return 0;
}

/**
* \brief Load and init simulation geometry and initialize threads after previous sanity check
 * Setup inidividual particleTracers per thread and local desorption limits
 * \return true on error, false when ok
 */
bool SimulationController::Load() {
    DEBUG_PRINT("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
    SetThreadStates(PROCESS_STARTING, "Loading simulation");

    auto sane = simulation->SanityCheckModel(false);
    if(!sane.first) {
        SetThreadStates(PROCESS_STARTING, "Loading simulation");
        bool loadError = false;

        // Init particleTracers / threads
        simulation->SetNParticle(nbThreads, false);
        if (simulation->LoadSimulation(procInfo->subProcInfos[0].slaveStatus)) {
            loadError = true;
        }

        if (!loadError) { // loadOk = Load();
            loadOk = true;

            {//if(nbThreads != simThreads.size()) {
                simThreads.clear();
                simThreads.reserve(nbThreads);
                for (size_t t = 0; t < nbThreads; t++) {
                    simThreads.emplace_back(
                            SimThread(procInfo, simulation, t));
                    simThreads.back().particleTracer = simulation->GetParticleTracer(t);
                }
            }

            // "Warm up" threads, to remove overhead for performance benchmarks
            double randomCounter = 0;
#pragma omp parallel default(none) shared(randomCounter)
            {
                double local_result = 0;
#pragma omp for
                for (int i=0; i < 1000; i++) {
                    local_result += 1;
                }
#pragma omp critical
                randomCounter += local_result;
            }
            DEBUG_PRINT("[OMP] Init: %zu\n", randomCounter);
            

            // Calculate remaining work
            size_t desPerThread = 0;
            size_t remainder = 0;
            size_t des_global = simulation->globState->globalStats.globalHits.nbDesorbed;
            if (des_global > 0) {
                desPerThread = des_global / nbThreads;
                remainder = des_global % nbThreads;
            }
            for (auto &thread : simThreads) {
                thread.particleTracer->totalDesorbed = desPerThread;
                thread.particleTracer->totalDesorbed += (thread.threadNum < remainder) ? 1 : 0;
            }

            SetRuntimeInfo();
        }
    }
    else {
        loadOk = false;
        SetThreadStates(PROCESS_ERROR, sane.second->c_str());
    }
    SetReady(loadOk);

    return !loadOk;
}

/**
* \brief Update on the fly parameters when called from the GUI
 * \return true on success
 */
bool SimulationController::UpdateParams() {
    // Load geometry
    auto* sim = simulation;
    if (sim->model->otfParams.enableLogging) {
        Log::console_msg(3, "Logging with size limit {}\n",
               sizeof(size_t) + sim->model->otfParams.logLimit * sizeof(ParticleLoggerItem));
    }
    sim->ReinitializeParticleLog();
    return true;
}

/**
* \brief Start the simulation after previous sanity checks
 * \return 0> error code, 0 when ok
 */
int SimulationController::Start() {

    // Check simulation model and geometry one last time
    auto sane = simulation->SanityCheckModel(true);
    if(sane.first){
        loadOk = false;
    }

    for(auto& thread : simThreads){
        if (!thread.particleTracer) {
            loadOk = false;
            break;
        }
    }

    if(!loadOk) {
        if(sane.second)
            SetThreadStates(PROCESS_ERROR, *sane.second);
        else
            SetThreadStates(PROCESS_ERROR, GetThreadStatuses());
        return 1;
    }

	if (simulation->model->accel.empty()) {
		loadOk = false;
		SetThreadStates(PROCESS_ERROR, "Failed building acceleration structure");
		return 1;
	}

    if (simulation->model->otfParams.desorptionLimit > 0) {
        if (simulation->totalDesorbed >=
            simulation->model->otfParams.desorptionLimit /
            simulation->model->otfParams.nbProcess) {
            ClearCommand();
        }
    }

    if (GetThreadStates() != PROCESS_RUN) {
        DEBUG_PRINT("[%d] COMMAND: START (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
        SetThreadStates(PROCESS_RUN, GetThreadStatuses());
    }

    if (loadOk) {
        procInfo->InitActiveProcList();

        // Calculate remaining work
        size_t desPerThread = 0;
        size_t remainder = 0;
        if(simulation->model->otfParams.desorptionLimit > 0){
            if(simulation->model->otfParams.desorptionLimit > (simulation->globState->globalStats.globalHits.nbDesorbed)) {
                size_t limitDes_global = simulation->model->otfParams.desorptionLimit;
                desPerThread = limitDes_global / nbThreads;
                remainder = limitDes_global % nbThreads;
            }
        }
        for(auto& thread : simThreads){
            size_t localDes = (desPerThread > thread.particleTracer->totalDesorbed) ? desPerThread - thread.particleTracer->totalDesorbed : 0; //remaining
            thread.localDesLimit = (thread.threadNum < remainder) ? localDes + 1 : localDes;
        }

        bool maxReachedOrDesError_global = false;
#pragma omp parallel num_threads(nbThreads)
        {
            bool maxReachedOrDesError_private = false;
            bool lastUpdateOk_private = true;

            // Set OpenMP thread priority on Windows whenever we start a simulation run
#if defined(_WIN32) && defined(_MSC_VER)
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_IDLE);
#endif
            maxReachedOrDesError_private = simThreads[omp_get_thread_num()].runLoop();

            if(maxReachedOrDesError_private) {
//#pragma omp atomic
                maxReachedOrDesError_global = true; //Race condition ok, maxReachedOrDesError_global means "at least one thread finished"
            }
        }

        ClearCommand();
        if (GetThreadStates() != PROCESS_ERROR) {
            if (maxReachedOrDesError_global) {
                DEBUG_PRINT("[%d] COMMAND: PROCESS_DONE (Max reached)\n", prIdx);
            }
            else {
                DEBUG_PRINT("[%d] COMMAND: PROCESS_DONE (Stopped)\n", prIdx);
            }
        }
    } else {
        SetErrorSub("No geometry loaded");
        ClearCommand();
    }
    return 0;
}

int SimulationController::Reset() {
    DEBUG_PRINT("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
    SetThreadStates(PROCESS_STARTING, "Resetting local cache...", false, true);
    resetControls();
    auto *sim = simulation;
    sim->ResetSimulation();
    SetReady(loadOk);

    return 0;
}

void SimulationController::EmergencyExit(){
    for(auto& t : simThreads)
        t.particleTracer->allQuit = true;
};