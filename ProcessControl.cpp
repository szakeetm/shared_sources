//
// Created by Pascal Baehr on 21.08.20.
//

#include "ProcessControl.h"
#include <string>

SubProcInfo::SubProcInfo() {
    procId = 0;
    slaveState = 0;
    masterCmd = 0;
    cmdParam = 0;
    cmdParam2 = 0;
    oldState = 0;
    memset(statusString, '\0', 128*sizeof(char));
    runtimeInfo = PROCESS_INFO();
}
/**
* \brief Assign operator
* \param src reference to source object
* \return address of this
*/
SubProcInfo& SubProcInfo::operator=(const SubProcInfo & src) {
    procId = src.procId;
    slaveState = src.slaveState;
    masterCmd = src.masterCmd;
    cmdParam = src.cmdParam;
    cmdParam2 = src.cmdParam2;
    oldState = src.oldState;
    std::strncpy(statusString,src.statusString,128);
    runtimeInfo = src.runtimeInfo;
    return *this;
}