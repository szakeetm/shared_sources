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

//! Moves first sub process to the back of the "active", for round robin fashion of communication for updates
void ProcComm::NextSubProc() {
    this->activeProcsMutex.lock();
    activeProcs.emplace_back(activeProcs.front());
    activeProcs.pop_front();
    this->activeProcsMutex.unlock();
}

//! Removes a process from the active list, in case it is finished
void ProcComm::RemoveAsActive(size_t id) {
    this->activeProcsMutex.lock();
    for(auto proc = activeProcs.begin(); proc != activeProcs.end(); ++proc){
        if(id == (*proc)) {
            activeProcs.erase(proc);
            break;
        }
    }
    this->activeProcsMutex.unlock();
}

//! Init list of active/simulating processes
void ProcComm::InitActiveProcList() {
    this->activeProcsMutex.lock();
    activeProcs.clear();
    for(size_t id = 0; id < this->subProcInfos.size(); ++id)
        activeProcs.emplace_back(id);
    this->activeProcsMutex.unlock();
}

