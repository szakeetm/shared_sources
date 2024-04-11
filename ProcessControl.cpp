

#include "ProcessControl.h"
#include <cstring>
#include <optional>

//! Moves first sub process to the back of the "active", for round robin fashion of communication for updates
void ProcComm::PlaceFrontToBack() {
    //this->activeProcsMutex.lock();
    hitUpdateQueue.emplace_back(hitUpdateQueue.front());
    hitUpdateQueue.pop_front();
    //this->activeProcsMutex.unlock();
}

//! Removes a process from the active list, in case it is finished
void ProcComm::RemoveFromHitUpdateQueue(size_t id) {
    //this->activeProcsMutex.lock();
    for(auto proc = hitUpdateQueue.begin(); proc != hitUpdateQueue.end(); ++proc){
        if(id == (*proc)) {
            hitUpdateQueue.erase(proc);
            break;
        }
    }
    //this->activeProcsMutex.unlock();
}

//! Init list of active/simulating processes
void ProcComm::InitHitUpdateQueue() {
    //this->activeProcsMutex.lock();
    hitUpdateQueue.clear();
    for(size_t id = 0; id < this->threadInfos.size(); ++id)
        hitUpdateQueue.emplace_back(id);
    //this->activeProcsMutex.unlock();
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