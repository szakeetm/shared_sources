//
// Created by Pascal Baehr on 21.08.20.
//

#include "ProcessControl.h"
#include <cstring>

ProcComm::ProcComm() : m() {
    masterCmd = 0;
    cmdParam = 0;
    cmdParam2 = 0;
}
/**
* \brief Assign operator
* \param src reference to source object
* \return address of this
*/
ProcComm& ProcComm::operator=(const ProcComm & src) {
    masterCmd = src.masterCmd;
    cmdParam = src.cmdParam;
    cmdParam2 = src.cmdParam2;
    subProcInfo = src.subProcInfo;
    return *this;
}

/**
* \brief Assign operator
* \param src reference to source object
* \return address of this
*/
ProcComm& ProcComm::operator=(ProcComm && src) noexcept {
    masterCmd = src.masterCmd;
    cmdParam = src.cmdParam;
    cmdParam2 = src.cmdParam2;
    subProcInfo = std::move(src.subProcInfo);
    return *this;
}

void ProcComm::NextSubProc() {
    this->m.lock();
    activeProcs.emplace_back(activeProcs.front());
    activeProcs.pop_front();
    this->m.unlock();
}

void ProcComm::RemoveAsActive(size_t id) {
    this->m.lock();
    for(auto proc = activeProcs.begin(); proc != activeProcs.end(); ++proc){
        if(id == (*proc)) {
            activeProcs.erase(proc);
            break;
        }
    }
    this->m.unlock();
}

void ProcComm::InitActiveProcList() {
    this->m.lock();
    activeProcs.clear();
    for(size_t id = 0; id < this->subProcInfo.size(); ++id)
        activeProcs.emplace_back(id);
    this->m.unlock();
}
