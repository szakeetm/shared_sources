//
// Created by Pascal Baehr on 04.05.20.
//

#include <cmath>
#include <sstream>
#include "SimulationController.h"
#include "../src/Simulation.h"
#include "ProcessControl.h"
#include <omp.h>
#include <thread>

#if not defined(_MSC_VER)
#include <sys/time.h>
#include <sys/resource.h>
#endif



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#endif

#define WAITTIME    500  // Answer in STOP mode

SimThread::SimThread(ProcComm *procInfo, SimulationUnit *sim, size_t threadNum) {
    this->threadNum = threadNum;
    this->procInfo = procInfo;
    this->status = nullptr;
    simulation = sim;
    stepsPerSec = 1.0;
    particle = nullptr;
    simEos = false;
}

SimThread::~SimThread() = default;

bool SimThread::runLoop() {
    bool eos;
    bool lastUpdateOk = false;

    //double timeStart = omp_get_wtime();
    do {
        setSimState(getSimStatus());
        simEos = runSimulation();      // Run during 1 sec
        if (procInfo->currentSubProc == threadNum) {
            size_t timeOut = lastUpdateOk ? 0 : 100; //ms
            lastUpdateOk = particle->UpdateHits(simulation->globState,
                                                timeOut); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
            procInfo->NextSubProc();
        }
        else{
            lastUpdateOk = false;
        }
        eos = simEos || (procInfo->masterCmd != COMMAND_START) || (procInfo->subProcInfo[threadNum].slaveState == PROCESS_ERROR);
    } while (/*omp_get_wtime() - timeStart > 20.0 && */!eos);

    if (!lastUpdateOk) {
        setSimState("Final update...");
        particle->UpdateHits(simulation->globState,
                             20000); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).)
    }
    return simEos;
}

void SimThread::setSimState(char *msg) const {
    snprintf(procInfo->subProcInfo[threadNum].statusString, 128, "%s", msg);
}

[[nodiscard]] char *SimThread::getSimStatus() const {
    static char ret[128];
    size_t count = simulation->globState->globalHits.globalHits.hit.nbDesorbed;
    size_t max = 0;
    if (simulation->model.otfParams.nbProcess)
        max = simulation->model.otfParams.desorptionLimit / simulation->model.otfParams.nbProcess;

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        sprintf(ret, "(%s) MC %zd/%zd (%.1f%%)", simulation->model.sh.name.c_str(), count, max, percent);
    } else {
        sprintf(ret, "(%s) MC %zd", simulation->model.sh.name.c_str(), count);
    }
    return ret;
}

int SimThread::runSimulation() {
    // 1s step
    size_t nbStep = (stepsPerSec <= 0.0) ? 250 : std::ceil(stepsPerSec + 0.5);

    {
        char msg[128];
        snprintf(msg, 128, "%s [%zu event/s]", getSimStatus(), nbStep);
        setSimState(msg);
    }

    //auto start_time = std::chrono::high_resolution_clock::now();
    double start_time = omp_get_wtime();
    bool goOn = particle->SimulationMCStep(nbStep, threadNum);
    double end_time = omp_get_wtime();
    //auto end_time = std::chrono::high_resolution_clock::now();


    if (goOn) // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
    {
        const double elapsedTimeMs = (end_time - start_time); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs != 0.0)
            stepsPerSec = (1.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run
    }

#if defined(_DEBUG)
    printf("Running: stepPerSec = %lf [%lu]\n", stepsPerSec, threadNum);
#endif

    return !goOn;
}

SimulationController::SimulationController(const std::string &appName, size_t parentPID, size_t procIdx,
                                           size_t nbThreads,
                                           std::vector<SimulationUnit *> *simulationInstance, ProcComm *pInfo)
        : appName{} {
    this->prIdx = procIdx;
    this->parentPID = parentPID;
    if (nbThreads == 0)
        this->nbThreads = omp_get_max_threads();
    else
        this->nbThreads = nbThreads;

    sprintf(this->appName, "%s", appName.c_str());

    simulation = simulationInstance; // TODO: Find a nicer way to manager derived simulationunit for Molflow and Synrad
    procInfo = pInfo; // Set from outside to link with general structure

    loadOK = false;
    resetControls();

    SetRuntimeInfo();

    printf("Controller created (%zd bytes)\n", sizeof(SHCONTROL));
    SetReady(false);
}

SimulationController::~SimulationController() = default;

SimulationController::SimulationController(SimulationController &&o) noexcept: appName{} {
    sprintf(this->appName, "%s", o.appName);

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
    loadOK = o.loadOK;
}

int SimulationController::resetControls() {
    lastHitUpdateOK = true;
    endState = false;

    stepsPerSec = 0.0;

    return 0;
}

int SimulationController::StartSimulation() {
    try {
        for (auto &sim : *simulation)
            sim->SanityCheckGeom();
    }
    catch (std::runtime_error &e) {
        return 1;
    }

    SetState(PROCESS_RUN, GetSimuStatus());
    //if (!currentParticle.lastHitFacet) StartFromSource();
    //return (currentParticle.lastHitFacet != nullptr);

    return 0;
}

int SimulationController::RunSimulation() {
    abort();
}

int SimulationController::SetRuntimeInfo() {

    // Update runtime information
    //procInfo->procId = 0; // TODO: There is no more procId
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    //procInfo->procId = _getpid();
#else
    //procInfo->procId = ::getpid();
#endif //  WIN

    for (auto &pInfo : procInfo->subProcInfo)
        GetProcInfo(pInfo.procId, &pInfo.runtimeInfo);

    return 0;
}

int SimulationController::ClearCommand() {

    procInfo->masterCmd = COMMAND_NONE;
    procInfo->cmdParam = 0;
    procInfo->cmdParam2 = 0;
    for (auto &pInfo : procInfo->subProcInfo) {
        strncpy(pInfo.statusString, GetSimuStatus(), 127);
        pInfo.statusString[127] = '\0';
    }
    return 0;
}

int SimulationController::SetState(size_t state, const char *status, bool changeState, bool changeStatus) {

    if (changeState) {
        printf("\n setstate %zd \n", state);
        //master->procInformation[prIdx].masterCmd = state;
        for (auto &pInfo : procInfo->subProcInfo) {
            pInfo.slaveState = state;
        }
    }
    if (changeStatus) {
        for (auto &pInfo : procInfo->subProcInfo) {
            strncpy(pInfo.statusString, status, 127);
            pInfo.statusString[127] = '\0';
        }
    }

    return 0;
}

char *SimulationController::GetSimuStatus() {
    static char ret[128];
    size_t count = 0;
    if(simulation->empty() || !simulation->at(0)->globState) {
        sprintf(ret, "[NONE]");
        return ret;
    }
    auto& sim = simulation->at(0);
    count = sim->globState->globalHits.globalHits.hit.nbDesorbed;
    size_t max = 0;
    if (sim->model.otfParams.nbProcess)
        max = sim->model.otfParams.desorptionLimit / sim->model.otfParams.nbProcess;

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        sprintf(ret, "(%s) MC %zd/%zd (%.1f%%)", sim->model.sh.name.c_str(), count, max, percent);
    } else {
        sprintf(ret, "(%s) MC %zd", sim->model.sh.name.c_str(), count);
    }

    return ret;
}

void SimulationController::SetErrorSub(const char *message) {
    printf("Error: %s\n", message);
    SetState(PROCESS_ERROR, message);
}

void SimulationController::SetStatus(char *status) {
    for (auto &pInfo : procInfo->subProcInfo) {
        strncpy(pInfo.statusString, status, 127);
        pInfo.statusString[127] = '\0';
    }
}

void SimulationController::SetReady(const bool loadOk) {

    if (loadOk)
        SetState(PROCESS_READY, this->GetSimuStatus());
    else
        SetState(PROCESS_READY, "(No geometry)");

    ClearCommand();
}

// Return Error code if subproc info is out of sync
size_t SimulationController::GetLocalState() const {
    size_t locState = procInfo->subProcInfo[0].slaveState;
    for (auto &pInfo : procInfo->subProcInfo) {
        if (locState != pInfo.slaveState)
            return PROCESS_ERROR;
    }
    return locState;
}

// Main loop
int SimulationController::controlledLoop(int argc, char **argv) {
    endState = false;
    bool loadOk = false;
    while (!endState) {
        bool eos = false;
        switch (procInfo->masterCmd) {

            case COMMAND_LOAD: {
                printf("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetState(PROCESS_STARTING, "Loading simulation");
                loadOk = Load();
                if (loadOk) {
                    //desorptionLimit = procInfo->cmdParam2; // 0 for endless
                    SetRuntimeInfo();
                }
                SetReady(loadOk);

                break;
            }
            case COMMAND_UPDATEPARAMS: {
                SetState(PROCESS_WAIT, GetSimuStatus());
                printf("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (UpdateParams()) {
                    SetReady(loadOk);
                    //SetState(procInfo->cmdParam2, GetSimuStatus());
                } else {
                    SetState(PROCESS_ERROR, "Could not update parameters");
                }
                break;
            }
            case COMMAND_RELEASEDPLOG: {
                SetState(PROCESS_WAIT, GetSimuStatus());
                printf("[%d] COMMAND: RELEASEDPLOG (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetReady(loadOk);
                break;
            }
            case COMMAND_START: {
                // Check end of simulation
                if ((*simulation)[0]->model.otfParams.desorptionLimit > 0) {
                    if ((*simulation)[0]->totalDesorbed >=
                        (*simulation)[0]->model.otfParams.desorptionLimit /
                        (*simulation)[0]->model.otfParams.nbProcess) {
                        ClearCommand();
                        SetState(PROCESS_DONE, GetSimuStatus());
                    }
                }
                if (GetLocalState() != PROCESS_RUN) {
                    printf("[%d] COMMAND: START (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                    SetState(PROCESS_RUN, GetSimuStatus());
                }
                bool lastUpdateOk = true;
                if (loadOk) {
                    size_t updateThread = 0;
                    /*SimThread simThread(&procInfo->slaveState, &procInfo->masterCmd, simulation);
                    simThread.runLoop(this->prIdx);*/
                    //nbThreads = 12;
                    //this->simulation->model.m.lock();
                    std::vector<std::thread> threads = std::vector<std::thread>(nbThreads - 1);
                    std::vector<SimThread> simThreads;
                    simThreads.reserve(nbThreads);
                    for (size_t t = 0; t < nbThreads; t++) {
                        simThreads.emplace_back(
                                SimThread(procInfo, simulation->at(t), t));
                        simThreads.back().particle = simulation->at(t)->GetParticle();
                    }
                    /*size_t threadNum = 0;
                    for(auto& thr : threads){
                        thr = std::thread(&SimThread::runLoop,&simThreads[threadNum],threadNum); //Launch main loop
                        threadNum++;
                    }

                    simThreads[threadNum].runLoop(threadNum);
                    for(auto& thread : threads){
                        thread.join();
                    }*/
                    //this->simulation->model.m.unlock();
                    bool simuEnd = false;
#pragma omp parallel num_threads(nbThreads) default(none) firstprivate(/*stepsPerSec,*/ lastUpdateOk, eos) shared(/*procInfo,*/ updateThread, simuEnd, /*simulation,*/simThreads)
                    {
                        eos = simThreads[omp_get_thread_num()].runLoop();

#pragma omp atomic
                        simuEnd |= eos;
                        /*do {
                            eos = RunSimulation();      // Run during 1 sec
                            if (updateThread == omp_get_thread_num() && (GetLocalState() != PROCESS_ERROR)) {
                                size_t timeOut = lastUpdateOk ? 20 : 100; //ms
                                lastUpdateOk = simulation->UpdateHits(omp_get_thread_num(),
                                                                      timeOut); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
                                updateThread = (updateThread+1) % omp_get_num_threads();
                            }
                        } while (procInfo->masterCmd == COMMAND_START && !eos);*/
                    }

                    if (simuEnd) {
                        if (GetLocalState() != PROCESS_ERROR) {
                            // Max desorption reached
                            ClearCommand();
                            SetState(PROCESS_DONE, GetSimuStatus());
                            printf("[%d] COMMAND: PROCESS_DONE (Max reached)\n", prIdx);
                        }
                    }
                    break;
                } else {
                    SetErrorSub("No geometry loaded");
                    ClearCommand();
                }
                break;
            }
            case COMMAND_PAUSE: {
                printf("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (!lastHitUpdateOK) {
                    // Last update not successful, retry with a longer timeout
                    if ((GetLocalState() != PROCESS_ERROR)) {
                        SetState(PROCESS_STARTING, "Updating hits...", false, true);
                        //lastHitUpdateOK = (*simulation)[0]->UpdateHits(prIdx, 60000);
                        SetState(PROCESS_STARTING, GetSimuStatus(), false, true);
                    }
                }
                SetReady(loadOk);
                break;
            }
            case COMMAND_RESET: {
                printf("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetState(PROCESS_STARTING, "Resetting local cache...", false, true);
                resetControls();
                for (auto &sim : *simulation)
                    sim->ResetSimulation();
                SetReady(loadOk);
                break;
            }
            case COMMAND_EXIT: {
                printf("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                endState = true;
                break;
            }
            case COMMAND_CLOSE: {
                printf("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                for (auto &sim : *simulation)
                    sim->ClearSimulation();
                loadOk = false;
                SetReady(loadOk);
                break;
            }
            case PROCESS_RUN: {
                eos = RunSimulation();      // Run during 1 sec
                if ((GetLocalState() != PROCESS_ERROR)) {
                    //lastHitUpdateOK = (*simulation)[0]->UpdateHits(prIdx,20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
                }
                if (eos) {
                    if (GetLocalState() != PROCESS_ERROR) {
                        // Max desorption reached
                        ClearCommand();
                        SetState(PROCESS_DONE, GetSimuStatus());
                        printf("[%d] COMMAND: PROCESS_DONE (Max reached)\n", prIdx);
                    }
                }
                break;
            }
            default: {
                ProcessSleep(WAITTIME);
                break;
            }
        }
    }
    SetState(PROCESS_KILLED, "Process terminated peacefully");
    return 0;
}

bool SimulationController::Load() {
    SetState(PROCESS_STARTING, "Loading simulation");
    bool loadError = false;
    // Load geometry

    for (auto & sim : *simulation) {
        if (sim->model.vertices3.empty()) {
            char err[512];
            sprintf(err, "Loaded empty 'geometry' (%zd Bytes)", procInfo->cmdParam);
            SetErrorSub(err);
            loadError = true;
            return loadError;
        }
    }

#pragma omp parallel for default(none) shared(loadError)
    for (size_t i = 0; i < simulation->size(); ++i) {
        if (simulation->at(i)->LoadSimulation(procInfo->subProcInfo[i].statusString)) {
#pragma omp critical
            loadError = true;
        }
    }
    return !loadError;
}

bool SimulationController::UpdateParams() {

    // Load geometry

    //bool result = simulation->UpdateOntheflySimuParams(loader);
    for (auto &sim : *simulation) {
        if (sim->model.otfParams.enableLogging) {
            printf("Logging with size limit %zd\n",
                   sizeof(size_t) + sim->model.otfParams.logLimit * sizeof(ParticleLoggerItem));
        }
        sim->ReinitializeParticleLog();
    }
    return true;
}