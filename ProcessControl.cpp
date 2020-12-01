//
// Created by Pascal Baehr on 21.08.20.
//

#include "ProcessControl.h"
#include <cstring>

ProcComm::ProcComm() : m() {
    masterCmd = 0;
    cmdParam = 0;
    cmdParam2 = 0;
    currentSubProc = 0;
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
    currentSubProc = src.currentSubProc;
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
    currentSubProc = src.currentSubProc;
    subProcInfo = std::move(src.subProcInfo);
    return *this;
}

void ProcComm::NextSubProc() {
    this->m.lock();
    currentSubProc = (currentSubProc+1) % subProcInfo.size();
    this->m.unlock();
}
