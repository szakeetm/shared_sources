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
#include <Helper/OutputHelper.h>

#include <omp.h>

#if not defined(_MSC_VER)
#include <sys/time.h>
#include <sys/resource.h>
#endif



#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#include <process.h>
#endif

#define WAITTIME    500  // Answer in STOP mode

SimulationController::SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                                           SimulationUnit *simulationInstance, std::shared_ptr<ProcComm> pInfo) {
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
}

int SimulationController::resetControls() {
    lastHitUpdateOK = true;
    endState = false;

    stepsPerSec = 0.0;

    return 0;
}

int SimulationController::SetRuntimeInfo() {

    // Update runtime information
    for (auto &pInfo : procInfo->subProcInfo)
        GetProcInfo(pInfo.procId, &pInfo.runtimeInfo);

    return 0;
}

int SimulationController::ClearCommand() {

    procInfo->masterCmd = COMMAND_NONE;
    procInfo->cmdParam = 0;
    procInfo->cmdParam2 = 0;
    SetState(PROCESS_READY,GetSimuStatus());

    return 0;
}

int SimulationController::SetState(size_t state, const char *status, bool changeState, bool changeStatus) {

    if (changeState) {
        DEBUG_PRINT("setstate %zd \n", state);
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

    if(state == PROCESS_ERROR){
        procInfo->masterCmd = PROCESS_WAIT;
    }
    return 0;
}

int SimulationController::SetState(size_t state, const std::vector<std::string> &status, bool changeState, bool changeStatus) {

    if (changeState) {
        DEBUG_PRINT("setstate %zd \n", state);
        //master->procInformation[prIdx].masterCmd = state;
        for (auto &pInfo : procInfo->subProcInfo) {
            pInfo.slaveState = state;
        }
    }
    if (changeStatus) {
        if(procInfo->subProcInfo.size() != status.size()){
            for (auto &pInfo : procInfo->subProcInfo) {
                strncpy(pInfo.statusString, "invalid state", 127);
                pInfo.statusString[127] = '\0';
            }
        }
        else {
            size_t pInd = 0;
            for (auto &pInfo : procInfo->subProcInfo) {
                strncpy(pInfo.statusString, status[pInd++].c_str(), 127);
                pInfo.statusString[127] = '\0';
            }
        }
    }

    if(state == PROCESS_ERROR){
        procInfo->masterCmd = PROCESS_WAIT;
    }
    return 0;
}

std::vector<std::string> SimulationController::GetSimuStatus() {

    std::vector<std::string> ret_vec(nbThreads);
    if (!simulation) {
        ret_vec.assign(nbThreads, "[NONE]");
    }
    else{
        size_t threadId = 0;
        auto* sim = simulation;
        for(size_t p = 0; p < nbThreads; ++p) {
            auto* particle = (simulation)->GetParticle(p);
            if(particle == nullptr) break;
            if(!particle->tmpState.initialized){
                ret_vec[threadId]= "[NONE]";
            }
            else {
                size_t count = 0;
                count = particle->totalDesorbed + particle->tmpState.globalHits.globalHits.nbDesorbed;
                size_t max = 0;
                if (sim->model->otfParams.nbProcess)
                    max = sim->model->otfParams.desorptionLimit / sim->model->otfParams.nbProcess + ((threadId < sim->model->otfParams.desorptionLimit % sim->model->otfParams.nbProcess) ? 1 : 0);

                char tmp[128];
                if (max != 0) {
                    double percent = (double) (count) * 100.0 / (double) (max);
                    sprintf(tmp, "MC %zd/%zd (%.1f%%)", count, max, percent);
                } else {
                    sprintf(tmp, "MC %zd", count);
                }
                ret_vec[threadId] = tmp;
            }
            ++threadId;
        }
    }
    return ret_vec;
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
    loadOk = false;
    while (!endState) {
        bool eos = false;
        switch (procInfo->masterCmd) {

            case COMMAND_LOAD: {
                DEBUG_PRINT("[%d] COMMAND: LOAD (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if(Load()){
                    SetState(PROCESS_ERROR, "Could not load simulation");
                }
                break;
            }
            case COMMAND_UPDATEPARAMS: {
                SetState(PROCESS_WAIT, GetSimuStatus());
                DEBUG_PRINT("[%d] COMMAND: UPDATEPARAMS (%zd,%zd)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (UpdateParams()) {
                    SetReady(loadOk);
                    //SetState(procInfo->cmdParam2, GetSimuStatus());
                } else {
                    SetState(PROCESS_ERROR, "Could not update parameters");
                }
                break;
            }
            case COMMAND_START: {
                Start();
                break;
            }
            case COMMAND_PAUSE: {
                DEBUG_PRINT("[%d] COMMAND: PAUSE (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
                if (!lastHitUpdateOK) {
                    // Last update not successful, retry with a longer timeout
                    if ((GetLocalState() != PROCESS_ERROR)) {
                        SetState(PROCESS_STARTING, "Updating hits...", false, true);
                        //lastHitUpdateOK = simulation->UpdateHits(prIdx, 60000);
                        SetState(PROCESS_STARTING, GetSimuStatus(), false, true);
                    }
                }
                SetReady(loadOk);
                break;
            }
            case COMMAND_RESET: {
                DEBUG_PRINT("[%d] COMMAND: RESET (%zd,%zu)\n", prIdx, procInfo->cmdParam, procInfo->cmdParam2);
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
                if(sim)
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
    SetState(PROCESS_KILLED, "Process terminated peacefully");
    return 0;
}

int SimulationController::RebuildAccel() {
    if (simulation->RebuildAccelStructure()) {
        return 1;
    }
    return 0;
}

bool SimulationController::UpdateParams() {
    // Load geometry
    auto* sim = simulation;
    if (sim->model->otfParams.enableLogging) {
        printf("Logging with size limit %zd\n",
               sizeof(size_t) + sim->model->otfParams.logLimit * sizeof(ParticleLoggerItem));
    }
    sim->ReinitializeParticleLog();
    return true;
}

int SimulationController::Reset() {
    SetState(PROCESS_STARTING, "Resetting local cache...", false, true);
    resetControls();
    auto *sim = simulation;
    sim->ResetSimulation();
    SetReady(loadOk);

    return 0;
}