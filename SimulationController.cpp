//
// Created by Pascal Baehr on 04.05.20.
//

#include <cmath>
#include <sstream>
#include <cereal/archives/binary.hpp>
#include "SimulationController.h"
#include "ProcessControl.h"
/*#if defined(MOLFLOW)
#include "../src/Simulation.h"
#endif*/

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#endif

#define WAITTIME    100  // Answer in STOP mode

SimulationController::SimulationController(std::string appName, std::string dpName, size_t parentPID,
                                           size_t procIdx, SimulationUnit *simulationInstance, SubProcInfo *pInfo) {
    this->prIdx = procIdx;
    this->parentPID = parentPID;

    sprintf(this->appName,"%s", appName.c_str());

    simulation = simulationInstance; // TODO: Find a nicer way to manager derived simulationunit for Molflow and Synrad
    procInfo = pInfo; // Set from outside to link with general structure

    SetRuntimeInfo();

    printf("Controller created (%zd bytes)\n", sizeof(SHCONTROL));
    SetReady(false);
}

SimulationController::~SimulationController(){
    //delete simulation; // doesn't manage it
}

SimulationController::SimulationController(SimulationController&& o) noexcept{
    sprintf(this->appName,"%s", o.appName);
    sprintf(this->appName,"%s", o.appName);
    sprintf(this->appName,"%s", o.appName);
    sprintf(this->appName,"%s", o.appName);
    sprintf(this->appName,"%s", o.appName);

    simulation = o.simulation;
    procInfo = o.procInfo;

    o.simulation = nullptr;
    o.procInfo = nullptr;

    stepsPerSec = o.stepsPerSec;
    prIdx = o.prIdx;
    parentPID = o.parentPID;
    endState = o.endState;
    lastHitUpdateOK = o.lastHitUpdateOK;
}

int SimulationController::StartSimulation() {
    try{
        simulation->SanityCheckGeom();
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


    double t0 = GetTick();
    bool goOn = simulation->SimulationMCStep(nbStep);
    double t1 = GetTick();

    if(goOn) // don't update on end, this will give a false ratio (SimMCStep could return actual steps instead of plain "false"
        stepsPerSec = (5.0 * nbStep) / (t1 - t0); // every 1.0 second

#if defined(_DEBUG)
    printf("Running: stepPerSec = %lf\n", stepsPerSec);
#endif

    return !goOn;
}

int SimulationController::SetRuntimeInfo() {

    // Update runtime information
    procInfo->procId = 0; // TODO: There is no more procId
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    //procInfo->procId = _getpid();
#else
    //procInfo->procId = ::getpid();
#endif //  WIN

    GetProcInfo(procInfo->procId,&procInfo->runtimeInfo);

    return 0;
}

int SimulationController::ClearCommand() {
    
    procInfo->masterCmd = COMMAND_NONE;
    procInfo->cmdParam = 0;
    procInfo->cmdParam2 = 0;
    strncpy(procInfo->statusString, GetSimuStatus(), 127);
    procInfo->statusString[127] = '\0';
    
    return 0;
}

int SimulationController::SetState(size_t state, const char *status, bool changeState, bool changeStatus) {

    if (changeState) {
        printf("\n setstate %zd \n", state);
        //master->procInformation[prIdx].masterCmd = state;
        procInfo->slaveState = state;
    }
    if (changeStatus) {
        strncpy(procInfo->statusString, status, 127);
        procInfo->statusString[127] = '\0';
    }
 
    return 0;
}

char *SimulationController::GetSimuStatus() {
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
    strncpy(procInfo->statusString, status, 127);
    procInfo->statusString[127] = '\0';

}

void SimulationController::SetReady(const bool loadOk) {

    if (loadOk)
        SetState(PROCESS_READY, this->GetSimuStatus());
    else
        SetState(PROCESS_READY, "(No geometry)");

    ClearCommand();
}

size_t SimulationController::GetLocalState() const {
    return procInfo->slaveState;
}

// Main loop
int SimulationController::controlledLoop(int argc, char **argv){
    endState = false;
    bool loadOk = false;
    while (!endState) {
        GetState();
        bool eos = false;
        switch (procInfo->masterCmd) {

            case COMMAND_LOAD:
                printf("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetState(PROCESS_STARTING, "Loading simulation");
                loadOk = Load();
                if (loadOk) {
                    //desorptionLimit = procInfo->cmdParam2; // 0 for endless
                    SetRuntimeInfo();
                }
                SetReady(loadOk);

                break;

            case COMMAND_UPDATEPARAMS:
                SetState(PROCESS_WAIT, GetSimuStatus());
                printf("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (UpdateParams()) {
                    SetReady(loadOk);
                    //SetState(procInfo->cmdParam2, GetSimuStatus());
                } else{
                    SetState(PROCESS_ERROR, "Could not update parameters");
                }
                break;

            case COMMAND_RELEASEDPLOG:
                SetState(PROCESS_WAIT, GetSimuStatus());
                printf("[%d] COMMAND: RELEASEDPLOG (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetReady(loadOk);
                break;

            case COMMAND_START:
                // Check end of simulation
                if (simulation->model.otfParams.desorptionLimit > 0) {
                    if (simulation->totalDesorbed >= simulation->model.otfParams.desorptionLimit / simulation->model.otfParams.nbProcess) {
                        ClearCommand();
                        SetState(PROCESS_DONE, GetSimuStatus());
                    }
                }
                if(GetLocalState() != PROCESS_RUN) {
                    printf("[%d] COMMAND: START (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                    SetState(PROCESS_RUN, GetSimuStatus());
                }
                if (loadOk) {
                    SetStatus(GetSimuStatus()); //update hits only
                    eos = RunSimulation();      // Run during 1 sec
                    if ((GetLocalState() != PROCESS_ERROR)) {
                        //simulation->UpdateHits(dpHit, dpLog, prIdx,20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
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

            case COMMAND_PAUSE:
                printf("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (!lastHitUpdateOK) {
                    // Last update not successful, retry with a longer timeout
                    if ((GetLocalState() != PROCESS_ERROR)) {
                        SetState(PROCESS_STARTING, "Updating hits...", false, true);
                        simulation->UpdateHits(prIdx, 60000);
                        SetState(PROCESS_STARTING, GetSimuStatus(), false, true);
                    }
                }
                SetReady(loadOk);
                break;

            case COMMAND_RESET:
                printf("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                SetState(PROCESS_STARTING, "Resetting local cache...", false, true);
                simulation->ResetSimulation();
                SetReady(loadOk);
                break;

            case COMMAND_EXIT:
                printf("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                endState = true;
                break;

            case COMMAND_CLOSE:
                printf("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                simulation->ClearSimulation();
                loadOk = false;
                SetReady(loadOk);
                break;

            case COMMAND_FETCH:
                printf("[%d] COMMAND: FETCH (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if(procInfo->procId == procInfo->cmdParam) {
                    SetState(PROCESS_STARTING,"Preparing to upload hits");
                    //simulation->UploadHits(dpHit, dpLog, prIdx, 10000);
                }
                SetReady(loadOk);
                break;

            case PROCESS_RUN:
                SetStatus(GetSimuStatus()); //update hits only
                eos = RunSimulation();      // Run during 1 sec
                if ((GetLocalState() != PROCESS_ERROR)) {
                    simulation->UpdateHits(prIdx, 20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
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

            default:
                ProcessSleep(WAITTIME);
                break;
        }
    }
    SetState(PROCESS_KILLED, "Process terminated peacefully");
    return 0;
}

bool SimulationController::Load() {

    // Load geometry
    if (simulation->model.vertices3.empty()) {
        char err[512];
        sprintf(err, "Loaded empty 'geometry' (%zd Bytes)", procInfo->cmdParam);
        SetErrorSub(err);
        return false;
    }

    SetState(PROCESS_STARTING, "Loading simulation");
    if (simulation->LoadSimulation()) {
        return false;
    }

    return true;
}

bool SimulationController::UpdateParams() {

    // Load geometry

    //bool result = simulation->UpdateOntheflySimuParams(loader);

    if (simulation->model.otfParams.enableLogging) {
        printf("Logging with size limit %zd\n", sizeof(size_t) + simulation->model.otfParams.logLimit * sizeof(ParticleLoggerItem));
    }

    //TODO: Move to Simulation method call
    simulation->ReinitializeParticleLog();

    return true;
}