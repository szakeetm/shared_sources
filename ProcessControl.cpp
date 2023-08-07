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

#include "ProcessControl.h"
#include <cstring>
#include <optional>

//! Moves first sub process to the back of the "active", for round robin fashion of communication for updates
void ProcComm::PlaceFrontToBack() {
    //this->activeProcsMutex.lock();
    activeProcs.emplace_back(activeProcs.front());
    activeProcs.pop_front();
    //this->activeProcsMutex.unlock();
}

//! Removes a process from the active list, in case it is finished
void ProcComm::RemoveAsActive(size_t id) {
    //this->activeProcsMutex.lock();
    for(auto proc = activeProcs.begin(); proc != activeProcs.end(); ++proc){
        if(id == (*proc)) {
            activeProcs.erase(proc);
            break;
        }
    }
    //this->activeProcsMutex.unlock();
}

//! Init list of active/simulating processes
void ProcComm::InitActiveProcList() {
    //this->activeProcsMutex.lock();
    activeProcs.resize(threadInfos.size());
    stepsSinceUpdate.resize(threadInfos.size());
    for (size_t id = 0; id < this->threadInfos.size(); ++id) {
        activeProcs.emplace_back(id);
        stepsSinceUpdate[id] = id;
    }
}

ProcCommData& ProcCommData::operator=(const ProcCommData& other) {
    if (this == &other) {
        return *this; // Check for self-assignment
    }

    // Copy all members except the mutex
    masterCmd = other.masterCmd;
    cmdParam = other.cmdParam;
    cmdParam2 = other.cmdParam2;
    controllerStatus = other.controllerStatus;
    controllerState = other.controllerState;
    threadInfos = other.threadInfos;
    stepsSinceUpdate = other.stepsSinceUpdate;

    // No need to copy the mutex, it's not copyable

    return *this;
}

void ProcCommData::UpdateCounterSizes(const std::vector<size_t>& counterSizes) {
    procDataMutex.lock();
    for (int i = 0; i < std::min(counterSizes.size(), threadInfos.size()); i++) {
        threadInfos[i].runtimeInfo.counterSize = counterSizes[i];
    }
    procDataMutex.unlock();
}

void ProcCommData::UpdateControllerStatus(const std::optional<ControllerState>& state, const std::optional<std::string>& status, LoadStatus_abstract* loadStatus) {
    procDataMutex.lock();
    if (status.has_value()) controllerStatus = *status;
    if (state.has_value()) controllerState = *state;
    procDataMutex.unlock();
    if (loadStatus) {
        loadStatus->procStateCache.procDataMutex.lock();
        if (status.has_value()) loadStatus->procStateCache.controllerStatus = *status;
        if (state.has_value()) loadStatus->procStateCache.controllerState = *state;
        loadStatus->procStateCache.procDataMutex.unlock();
        loadStatus->Update();
    }
};