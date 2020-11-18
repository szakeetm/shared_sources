//
// Created by Pascal Baehr on 04.05.20.
//

#include <cmath>
#include <sstream>
#include <cereal/archives/binary.hpp>
#include "SimulationController.h"
#include "../src/Simulation.h"
#include "ProcessControl.h"
#include <omp.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <thread>
/*#if defined(MOLFLOW)
#include "../src/Simulation.h"
#endif*/

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#endif

#define WAITTIME    100  // Answer in STOP mode

SimThread::SimThread(SubProcInfo* procInfo, SimulationUnit* simu){
    this->procInfo = procInfo;
    this->status = nullptr;
    simulation = simu;
    stepsPerSec= 1.0;
}
SimThread::~SimThread()= default;

bool SimThread::runLoop(size_t threadNum) {
    bool eos = false;
    bool lastUpdateOk = true;

    //double timeStart = omp_get_wtime();
    do {
        std::cout << "Thread #" << threadNum << ": on CPU " << sched_getcpu() << "\n";
        eos = runSimulation(threadNum);      // Run during 1 sec
        //if ((*slaveState != PROCESS_ERROR)) {
            size_t timeOut = lastUpdateOk ? 0 : 100; //ms
            lastUpdateOk = particle->UpdateHits(simulation->globState,
                                                  timeOut); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
        //}
        eos &= procInfo->masterCmd != COMMAND_START;
    } while (/*omp_get_wtime() - timeStart > 20.0 && */!eos);
    return eos;
}

[[nodiscard]] char *SimThread::getSimuStatus() const {
    static char ret[128];
    size_t count = simulation->totalDesorbed;
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
int SimThread::runSimulation(size_t threadNum) {
    // 1s step
    size_t    nbStep = 1;
    if (stepsPerSec <= 0.0) {
        nbStep = 250;
    }
    else {
        nbStep = std::ceil(stepsPerSec + 0.5);
    }

    {
        //snprintf(*status, 128, "%s [%zu event/s]", getSimuStatus(), nbStep);
    }

    //auto start_time = std::chrono::high_resolution_clock::now();
    double start_time = omp_get_wtime();
    bool goOn = particle->SimulationMCStep(nbStep, threadNum);
    double end_time = omp_get_wtime();
    //auto end_time = std::chrono::high_resolution_clock::now();


    if(goOn) // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
    {
        const double elapsedTimeMs = (end_time - start_time); //std::chrono::duration<float, std::ratio<1, 1>>(end_time - start_time).count();
        if (elapsedTimeMs != 0.0)
            stepsPerSec = (1.0 * nbStep) / (elapsedTimeMs); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run
    }

//#if defined(_DEBUG)
    printf("Running: stepPerSec = %lf [%lu]\n", stepsPerSec, threadNum);
//#endif

    return !goOn;
}

SimulationController::SimulationController(const std::string &appName, size_t parentPID, size_t procIdx, size_t nbThreads,
                                           std::vector<SimulationUnit*>*simulationInstance, std::vector<SubProcInfo>*pInfo) : appName{}{
    this->prIdx = procIdx;
    this->parentPID = parentPID;
    if(nbThreads == 0)
        this->nbThreads = omp_get_max_threads();
    else
        this->nbThreads = nbThreads;

    sprintf(this->appName,"%s", appName.c_str());

    simulation = simulationInstance; // TODO: Find a nicer way to manager derived simulationunit for Molflow and Synrad
    procInfo = pInfo; // Set from outside to link with general structure

    loadOK = false;
    resetControls();

    SetRuntimeInfo();

    printf("Controller created (%zd bytes)\n", sizeof(SHCONTROL));
    SetReady(false);
}

SimulationController::~SimulationController(){
    //delete simulation; // doesn't manage it
}

SimulationController::SimulationController(SimulationController&& o) noexcept : appName{}{
    sprintf(this->appName,"%s", o.appName);

    simulation = o.simulation;
    procInfo = o.procInfo;

    o.simulation = nullptr;
    o.procInfo = nullptr;

    stepsPerSec = o.stepsPerSec;
    endState = o.endState;
    lastHitUpdateOK = o.lastHitUpdateOK;

    prIdx = o.prIdx;
    parentPID = o.parentPID;
    nbThreads =  o.nbThreads;
    loadOK = o.loadOK;
}

int SimulationController::resetControls(){
    lastHitUpdateOK = true;
    endState = false;

    stepsPerSec = 0.0;

    return 0;
}

int SimulationController::StartSimulation() {
    try{
        for(auto& sim : *simulation)
            sim->SanityCheckGeom();
    }
    catch (std::runtime_error& e) {
        return 1;
    }

    SetState(PROCESS_RUN, GetSimuStatus());
    //if (!currentParticle.lastHitFacet) StartFromSource();
    //return (currentParticle.lastHitFacet != nullptr);

    return 0;
}

int SimulationController::RunSimulation() {
    // 1s step
    size_t    nbStep = 1;
    if (stepsPerSec <= 0.0) {
        nbStep = 250;
    }
    else {
        nbStep = std::ceil(stepsPerSec + 0.5);
    }

    {
        char tmp[128];
        snprintf(tmp, 128, "%s [%zu event/s]", GetSimuStatus(), nbStep);
        SetStatus(tmp); //update hits only
    }
    double t0 = omp_get_wtime();
    bool goOn = false;//(*simulation)[0]->currentParticle.SimulationMCStep(nbStep, 0);
    double t1 = omp_get_wtime();

    if(goOn) // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
    {
        if (t1 - t0 != 0.0)
            stepsPerSec = (1.0 * nbStep) / (t1 - t0); // every 1.0 second
        else
            stepsPerSec = (100.0 * nbStep); // in case of fast initial run
    }

//#if defined(_DEBUG)
    printf("Running: stepPerSec = %lf\n", stepsPerSec);
//#endif

    return !goOn;
}

int SimulationController::SetRuntimeInfo() {

    // Update runtime information
    (*procInfo)[0].procId = 0; // TODO: There is no more procId
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    //procInfo->procId = _getpid();
#else
    //procInfo->procId = ::getpid();
#endif //  WIN

    GetProcInfo((*procInfo)[0].procId,&(*procInfo)[0].runtimeInfo);

    return 0;
}

int SimulationController::ClearCommand() {

    (*procInfo)[0].masterCmd = COMMAND_NONE;
    (*procInfo)[0].cmdParam = 0;
    (*procInfo)[0].cmdParam2 = 0;
    strncpy((*procInfo)[0].statusString, GetSimuStatus(), 127);
    (*procInfo)[0].statusString[127] = '\0';

    return 0;
}

int SimulationController::SetState(size_t state, const char *status, bool changeState, bool changeStatus) {

    if (changeState) {
        printf("\n setstate %zd \n", state);
        //master->procInformation[prIdx].masterCmd = state;
        (*procInfo)[0].slaveState = state;
    }
    if (changeStatus) {
        strncpy((*procInfo)[0].statusString, status, 127);
        (*procInfo)[0].statusString[127] = '\0';
    }

    return 0;
}

char *SimulationController::GetSimuStatus() {
    static char ret[128];
    size_t count = (*simulation)[0]->totalDesorbed;
    size_t max = 0;
    if ((*simulation)[0]->model.otfParams.nbProcess)
        max = (*simulation)[0]->model.otfParams.desorptionLimit / (*simulation)[0]->model.otfParams.nbProcess;

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        sprintf(ret, "(%s) MC %zd/%zd (%.1f%%)", (*simulation)[0]->model.sh.name.c_str(), count, max, percent);
    } else {
        sprintf(ret, "(%s) MC %zd", (*simulation)[0]->model.sh.name.c_str(), count);
    }

    return ret;
}

void SimulationController::GetState() {
    //procInfo->masterCmd = PROCESS_READY;
    //procInfo->slaveState = PROCESS_READY;
    //procInfo->cmdParam = 0;

    if (!IsProcessRunning(parentPID)) {
        std::stringstream errMsg;
        errMsg << "Host "<<appName<<".exe (process id "<< parentPID << ") not running. Closing.";
        printf("%s\n",errMsg.str().c_str());
        SetErrorSub(errMsg.str().c_str());
        endState = true;
    }
}

void SimulationController::SetErrorSub(const char *message) {
    printf("Error: %s\n", message);
    SetState(PROCESS_ERROR, message);
}

void SimulationController::SetStatus(char *status) {
    strncpy((*procInfo)[0].statusString, status, 127);
    (*procInfo)[0].statusString[127] = '\0';

}

void SimulationController::SetReady(const bool loadOk) {

    if (loadOk)
        SetState(PROCESS_READY, this->GetSimuStatus());
    else
        SetState(PROCESS_READY, "(No geometry)");

    ClearCommand();
}

size_t SimulationController::GetLocalState() const {
    return (*procInfo)[0].slaveState;
}

// Main loop
int SimulationController::controlledLoop(int argc, char **argv){
    endState = false;
    bool loadOk = false;
    while (!endState) {
        GetState();
        bool eos = false;
        switch ((*procInfo)[0].masterCmd) {

            case COMMAND_LOAD: {
                printf("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
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
                printf("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
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
                printf("[%d] COMMAND: RELEASEDPLOG (%zd,%zd)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
                SetReady(loadOk);
                break;
            }
            case COMMAND_START: {
                // Check end of simulation
                if ((*simulation)[0]->model.otfParams.desorptionLimit > 0) {
                    if ((*simulation)[0]->totalDesorbed >=
                            (*simulation)[0]->model.otfParams.desorptionLimit / (*simulation)[0]->model.otfParams.nbProcess) {
                        ClearCommand();
                        SetState(PROCESS_DONE, GetSimuStatus());
                    }
                }
                if (GetLocalState() != PROCESS_RUN) {
                    printf("[%d] COMMAND: START (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
                    SetState(PROCESS_RUN, GetSimuStatus());
                }
                bool lastUpdateOk = true;
                if (loadOk) {
                    size_t updateThread = 0;
                    /*SimThread simThread(&(*procInfo)[0].slaveState, &(*procInfo)[0].masterCmd, simulation);
                    simThread.runLoop(this->prIdx);*/
                    nbThreads = 12;
                    //this->simulation->model.m.lock();
                    std::vector<std::thread> threads = std::vector < std::thread> (nbThreads - 1);
                    std::vector<SimThread> simThreads;
                    simThreads.reserve(nbThreads);
                    for(int t = 0; t < nbThreads; t++) {
                        simThreads.emplace_back(
                                SimThread(&procInfo->at(t), simulation->at(t)));
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

#pragma omp parallel num_threads(nbThreads) default(none) firstprivate(stepsPerSec, lastUpdateOk, eos) shared(procInfo, updateThread, simulation,simThreads)
                    {
                        simThreads[omp_get_thread_num()].runLoop(omp_get_thread_num());
                        /*do {
                            eos = RunSimulation();      // Run during 1 sec
                            if (updateThread == omp_get_thread_num() && (GetLocalState() != PROCESS_ERROR)) {
                                size_t timeOut = lastUpdateOk ? 20 : 100; //ms
                                lastUpdateOk = simulation->UpdateHits(omp_get_thread_num(),
                                                                      timeOut); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
                                updateThread = (updateThread+1) % omp_get_num_threads();
                            }
                        } while ((*procInfo)[0].masterCmd == COMMAND_START && !eos);*/
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
                } else {
                    SetErrorSub("No geometry loaded");
                    ClearCommand();
                }
                break;
            }
            case COMMAND_PAUSE: {
                printf("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
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
                printf("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
                SetState(PROCESS_STARTING, "Resetting local cache...", false, true);
                resetControls();
                (*simulation)[0]->ResetSimulation();
                SetReady(loadOk);
                break;
            }
            case COMMAND_EXIT: {
                printf("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
                endState = true;
                break;
            }
            case COMMAND_CLOSE: {
                printf("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
                (*simulation)[0]->ClearSimulation();
                loadOk = false;
                SetReady(loadOk);
                break;
            }
            case COMMAND_FETCH: {
                printf("[%d] COMMAND: FETCH (%zd,%zu)\n", prIdx, (*procInfo)[0].cmdParam, (*procInfo)[0].cmdParam2);
                if ((*procInfo)[0].procId == (*procInfo)[0].cmdParam) {
                    SetState(PROCESS_STARTING, "Preparing to upload hits");
                    //(*simulation)[0]->UploadHits(dpHit, dpLog, prIdx, 10000);
                }
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

    // Load geometry
    int i=0;
    for(auto& sim : *simulation) {
        if (sim->model.vertices3.empty()) {
            char err[512];
            sprintf(err, "Loaded empty 'geometry' (%zd Bytes)", (*procInfo)[i].cmdParam);
            SetErrorSub(err);
            return false;
        }


        SetState(PROCESS_STARTING, "Loading simulation");
        if (sim->LoadSimulation((*procInfo)[i++].statusString)) {
            return false;
        }
    }
    return true;
}

bool SimulationController::UpdateParams() {

    // Load geometry

    //bool result = simulation->UpdateOntheflySimuParams(loader);
    for(auto& sim : *simulation) {
        if (sim->model.otfParams.enableLogging) {
            printf("Logging with size limit %zd\n",
                   sizeof(size_t) + sim->model.otfParams.logLimit * sizeof(ParticleLoggerItem));
        }

        //TODO: Move to Simulation method call
        sim->ReinitializeParticleLog();
    }
    return true;
}