//
// Created by Pascal Baehr on 04.05.20.
//

#include <cmath>
#include <sstream>
#include <cereal/archives/binary.hpp>
#include "SimulationController.h"
#include "ProcessControl.h"

#define WAITTIME    100  // Answer in STOP mode

SimulationController::SimulationController(std::string appName , std::string dpName, size_t parentPID, size_t procIdx, SimulationUnit *simulationInstance){
    this->prIdx = procIdx;
    this->parentPID = parentPID;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    const char* dpPrefix = "";
#else
    const char *dpPrefix = "/"; // creates semaphore as /dev/sem/%s_sema
#endif
    sprintf(this->appName,"%s", appName.c_str());
    sprintf(this->ctrlDpName,"%s", std::string(dpPrefix+dpName+"CTRL"+std::to_string(parentPID)).c_str());
    sprintf(this->loadDpName,"%s", std::string(dpPrefix+dpName+"LOAD"+std::to_string(parentPID)).c_str());
    sprintf(this->hitsDpName,"%s", std::string(dpPrefix+dpName+"HITS"+std::to_string(parentPID)).c_str());
    sprintf(this->logDpName,"%s", std::string(dpPrefix+dpName+"LOG"+std::to_string(parentPID)).c_str());

    simulation = simulationInstance;
    loadOK = false;
    dpHit = nullptr;
    dpLog = nullptr;
    dpControl = OpenDataport(ctrlDpName, sizeof(SHCONTROL));
    if (!dpControl) {
        printf("Cannot connect to %s\n", ctrlDpName);
        exit(0);
    }

    printf("Connected to %s (%zd bytes), %sSub.exe #%d\n", ctrlDpName, sizeof(SHCONTROL), appName.c_str(), prIdx);
    SetReady();
};

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
        stepsPerSec = (1.0 * nbStep) / (t1 - t0); // every 1.0 second

#if defined(_DEBUG)
    printf("Running: stepPerSec = %lf\n", stepsPerSec);
#endif

    return !goOn;
}

int SimulationController::SetState(size_t state, const char *status, bool changeState, bool changeStatus) {

    procInfo.statusId = state;
    if (changeState) printf("\n setstate %zd \n", state);
    if (AccessDataport(this->dpControl)) {
        SHCONTROL *master = (SHCONTROL *) this->dpControl->buff;
        if (changeState) master->states[prIdx] = state;
        if (changeStatus) {
            strncpy(master->statusStr[prIdx], status, 127);
            master->statusStr[prIdx][127] = 0;
        }
        ReleaseDataport(this->dpControl);
    }
    return 0;
}

char *SimulationController::GetSimuStatus() {
    static char ret[128];
    size_t count = simulation->totalDesorbed;
    size_t max = 0;
    if (simulation->ontheflyParams.nbProcess)
        max = simulation->ontheflyParams.desorptionLimit / simulation->ontheflyParams.nbProcess;

    if (max != 0) {
        double percent = (double) (count) * 100.0 / (double) (max);
        sprintf(ret, "(%s) MC %zd/%zd (%.1f%%)", simulation->sh.name.c_str(), count, max, percent);
    } else {
        sprintf(ret, "(%s) MC %zd", simulation->sh.name.c_str(), count);
    }

    return ret;
}

void SimulationController::GetState() {
    procInfo.statusId = PROCESS_READY;
    procInfo.cmdParam = 0;

    if (AccessDataport(this->dpControl)) {
        auto master = (SHCONTROL *) this->dpControl->buff;
        procInfo.statusId = master->states[prIdx];
        procInfo.cmdParam = master->cmdParam[prIdx];
        procInfo.cmdParam2 = master->oldStates[prIdx];
        //master->cmdParam[prIdx] = 0;
        //master->oldStates[prIdx] = 0;

        ReleaseDataport(this->dpControl);

        if (!IsProcessRunning(parentPID)) {
            std::stringstream errMsg;
            errMsg << "Host "<<appName<<".exe (process id "<< parentPID << ") not running. Closing.";
            printf("%s\n",errMsg.str().c_str());
            SetErrorSub(errMsg.str().c_str());
            endState = true;
        }
    } else {
        printf("Subprocess %d couldn't connect to %s.\n", prIdx, appName);
        SetErrorSub("No connection to main program. Closing subprocess.");
        ProcessSleep(5000);
        endState = true;
    }
}

void SimulationController::SetErrorSub(const char *message) {
    printf("Error: %s\n", message);
    SetState(PROCESS_ERROR, message);
}

void SimulationController::SetStatus(char *status) {
    if (AccessDataport(this->dpControl)) {
        SHCONTROL *master = (SHCONTROL *) this->dpControl->buff;
        strncpy(master->statusStr[prIdx], status, 127);
        master->statusStr[prIdx][127] = 0;
        ReleaseDataport(this->dpControl);
    }
}

void SimulationController::SetReady() {

    if (this->loadOK)
        SetState(PROCESS_READY, this->GetSimuStatus());
    else
        SetState(PROCESS_READY, "(No geometry)");

}

size_t SimulationController::GetLocalState() const {
    return procInfo.statusId;
}

// Main loop
int SimulationController::controlledLoop(int argc, char **argv){
    endState = false;
    while (!endState) {
        GetState();
        bool eos = false;
        switch (procInfo.statusId) {

            case COMMAND_LOAD:
                printf("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                SetState(PROCESS_STARTING, "Loading simulation");
                loadOK = Load();
                if (loadOK) {
                    //desorptionLimit = procInfo.cmdParam2; // 0 for endless
                    SetReady();
                }
                break;

            case COMMAND_UPDATEPARAMS:
                printf("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                if (UpdateParams()) {
                    SetState(procInfo.cmdParam2, GetSimuStatus());
                }
                break;

            case COMMAND_RELEASEDPLOG:
                printf("[%d] COMMAND: RELEASEDPLOG (%zd,%zd)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                CLOSEDPSUB(dpLog);
                SetState(procInfo.cmdParam, GetSimuStatus());
                break;

            case COMMAND_START:
                printf("[%d] COMMAND: START (%zd,%zu)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                if (loadOK) {
                    if (!StartSimulation())
                        SetState(PROCESS_RUN, GetSimuStatus());
                    else {
                        if (GetLocalState() != PROCESS_ERROR)
                            SetState(PROCESS_DONE, GetSimuStatus());
                    }
                } else
                    SetErrorSub("No geometry loaded");
                break;

            case COMMAND_PAUSE:
                printf("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                if (!lastHitUpdateOK) {
                    // Last update not successful, retry with a longer timeout
                    if (dpHit && (GetLocalState() != PROCESS_ERROR)) {
                        SetState(PROCESS_STARTING, "Updating hits...", false, true);
                        simulation->UpdateHits(dpHit, dpLog, prIdx, 60000);
                        SetState(PROCESS_STARTING, GetSimuStatus(), false, true);
                    }
                }
                SetReady();
                break;

            case COMMAND_RESET:
                printf("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                SetState(PROCESS_STARTING, "Resetting local cache...", false, true);
                simulation->ResetSimulation();
                SetReady();
                break;

            case COMMAND_EXIT:
                printf("[%d] COMMAND: EXIT (%zd,%zu)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                endState = true;
                break;

            case COMMAND_CLOSE:
                printf("[%d] COMMAND: CLOSE (%zd,%zu)\n", prIdx, procInfo.cmdParam, procInfo.cmdParam2);
                loadOK = false;
                simulation->ClearSimulation();
                CLOSEDPSUB(dpHit);
                CLOSEDPSUB(dpLog);
                SetReady();
                break;

            case PROCESS_RUN:
                SetStatus(GetSimuStatus()); //update hits only
                eos = RunSimulation();      // Run during 1 sec
                if (dpHit && (GetLocalState() != PROCESS_ERROR)) {
                    simulation->UpdateHits(dpHit, dpLog, prIdx,
                               20); // Update hit with 20ms timeout. If fails, probably an other subprocess is updating, so we'll keep calculating and try it later (latest when the simulation is stopped).
                }
                if (eos) {
                    if (GetLocalState() != PROCESS_ERROR) {
                        // Max desorption reached
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

    Dataport *loader;
    size_t hSize;

    // Load geometry
    loader = OpenDataport(loadDpName, procInfo.cmdParam);
    if (!loader) {
        char err[512];
        sprintf(err, "Failed to connect to 'loader' dataport %s (%zd Bytes)", loadDpName, procInfo.cmdParam);
        SetErrorSub(err);
        return this->loadOK;
    }

    printf("Connected to %s\n", loadDpName);

    SetState(PROCESS_STARTING, "Loading simulation");
    if (!simulation->LoadSimulation(loader)) {
        CLOSEDPSUB(loader);
        return this->loadOK;
    }
    CLOSEDPSUB(loader);

    //Connect to log dataport
    if (simulation->ontheflyParams.enableLogging) {
        this->dpLog = OpenDataport(logDpName,
                                   sizeof(size_t) + simulation->ontheflyParams.logLimit * sizeof(ParticleLoggerItem));
        if (!this->dpLog) {
            char err[512];
            sprintf(err, "Failed to connect to 'this->dpLog' dataport %s (%zd Bytes)", logDpName,
                    sizeof(size_t) + simulation->ontheflyParams.logLimit * sizeof(ParticleLoggerItem));
            SetErrorSub(err);
            this->loadOK = false;
            return this->loadOK;
        }
        //*((size_t*)this->dpLog->buff) = 0; //Autofill with 0. Besides, we don't write without access!
    }

    // Connect to hit dataport
    hSize = simulation->GetHitsSize();
    this->dpHit = OpenDataport(hitsDpName, hSize);

    if (!this->dpHit) { // in case of unknown size, create the DP itself
        this->dpHit = CreateDataport(hitsDpName, hSize);
    }
    if (!this->dpHit) {
        char err[512];
        sprintf(err, "Failed to connect to 'hits' dataport (%zd Bytes)", hSize);
        SetErrorSub(err);
        this->loadOK = false;
        return this->loadOK;
    }

    printf("Connected to %s (%zd bytes)\n", hitsDpName, hSize);
    this->loadOK = true;

    return this->loadOK;
}

bool SimulationController::UpdateParams() {

    // Load geometry
    Dataport *loader = OpenDataport(loadDpName, procInfo.cmdParam);
    if (!loader) {
        char err[512];
        sprintf(err, "Failed to connect to 'loader' dataport %s (%zd Bytes)", loadDpName, procInfo.cmdParam);
        SetErrorSub(err);
        return false;
    }
    printf("Connected to %s\n", loadDpName);

    bool result = UpdateOntheflySimuParams(loader);
    CLOSEDPSUB(loader);

    if (simulation->ontheflyParams.enableLogging) {
        this->dpLog = OpenDataport(logDpName,
                                   sizeof(size_t) + simulation->ontheflyParams.logLimit * sizeof(ParticleLoggerItem));
        if (!this->dpLog) {
            char err[512];
            sprintf(err, "Failed to connect to 'this->dpLog' dataport %s (%zd Bytes)", logDpName,
                    sizeof(size_t) + simulation->ontheflyParams.logLimit * sizeof(ParticleLoggerItem));
            SetErrorSub(err);
            return false;
        }
        //*((size_t*)sHandle->dpLog->buff) = 0; //Autofill with 0, besides we would need access first
    }

    //TODO: Move to Simulation method call
    simulation->ReinitializeParticleLog();

    return result;
}

bool SimulationController::UpdateOntheflySimuParams(Dataport *loader) {
    // Connect the dataport


    if (!AccessDataportTimed(loader, 2000)) {
        SetErrorSub("Failed to connect to loader DP");
        return false;
    }
    std::string inputString(loader->size,'\0');
    BYTE* buffer = (BYTE*)loader->buff;
    std::copy(buffer, buffer + loader->size, inputString.begin());
    std::stringstream inputStream;
    inputStream << inputString;
    cereal::BinaryInputArchive inputArchive(inputStream);

    inputArchive(simulation->ontheflyParams);

    ReleaseDataport(loader);

    return true;
}