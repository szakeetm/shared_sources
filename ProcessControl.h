//
// Created by Pascal Baehr on 22.04.20.
//

#ifndef MOLFLOW_PROJ_PROCESSCONTROL_H
#define MOLFLOW_PROJ_PROCESSCONTROL_H

#include <cstddef> //size_t
#include <vector>
#include <mutex>

#define PROCESS_STARTING 0   // Loading state
#define PROCESS_RUN      1   // Running state
#define PROCESS_READY    2   // Waiting state
#define PROCESS_KILLED   3   // Process killed
#define PROCESS_ERROR    4   // Process in error
#define PROCESS_DONE     5   // Simulation ended
#define PROCESS_WAIT     6   // Command fully executed

#define COMMAND_NONE     10  // No change
#define COMMAND_LOAD     11  // Load geometry
#define COMMAND_START    12  // Start simu
#define COMMAND_PAUSE    13  // Pause simu
#define COMMAND_RESET    14  // Reset simu
#define COMMAND_EXIT     15  // Exit
#define COMMAND_CLOSE    16  // Release handles
#define COMMAND_UPDATEPARAMS 17 //Update simulation mode (low flux, fluxwise/powerwise, displayed regions)
#define COMMAND_RELEASEDPLOG 18 //Release dpLog handle (precedes Updateparams)

static const char *prStates[] = {

        "Not started",
        "Running",
        "Waiting",
        "Killed",
        "Error",
        "Done",
        "",
        "",
        "",
        "",
        "No command",
        "Loading",
        "Starting",
        "Stopping",
        "Resetting",
        "Exiting",
        "Closing",
        "Update params",
        "Release dpLog"
};

struct PROCESS_INFO{

    double cpu_time; // CPU time         (in second)
    size_t  mem_use;  // Memory usage     (in byte)
    size_t  mem_peak; // MAx Memory usage (in byte)

};

struct ProcComm {

    struct SubProcInfo {
        size_t procId;
        size_t slaveState;
        char statusString[128];
        PROCESS_INFO runtimeInfo;
    };

    size_t masterCmd;
    size_t cmdParam;
    size_t cmdParam2;
    size_t currentSubProc;
    std::mutex m;
    std::vector<SubProcInfo> subProcInfo;

    ProcComm();
    explicit ProcComm(size_t nbProcs) : ProcComm() {Resize(nbProcs);};
    void Resize(size_t nbProcs){subProcInfo.resize(nbProcs);};
    void NextSubProc();

    ProcComm& operator=(const ProcComm & src);
    ProcComm& operator=(ProcComm && src) noexcept ;
};

#endif //MOLFLOW_PROJ_PROCESSCONTROL_H
