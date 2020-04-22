//
// Created by Pascal Baehr on 22.04.20.
//

#ifndef MOLFLOW_PROJ_PROCESSCONTROL_H
#define MOLFLOW_PROJ_PROCESSCONTROL_H

// Master control shared memory block  (name: MFLWCTRL[masterPID])
//

#define PROCESS_STARTING 0   // Loading state
#define PROCESS_RUN      1   // Running state
#define PROCESS_READY    2   // Waiting state
#define PROCESS_KILLED   3   // Process killed
#define PROCESS_ERROR    4   // Process in error
#define PROCESS_DONE     5   // Simulation ended
#define PROCESS_RUNAC    6   // Computing AC matrix

#define COMMAND_NONE     10  // No change
#define COMMAND_LOAD     11  // Load geometry
#define COMMAND_START    12  // Start simu
#define COMMAND_PAUSE    13  // Pause simu
#define COMMAND_RESET    14  // Reset simu
#define COMMAND_EXIT     15  // Exit
#define COMMAND_CLOSE    16  // Release handles
#define COMMAND_UPDATEPARAMS 17 //Update simulation mode (low flux, fluxwise/powerwise, displayed regions)
#define COMMAND_RELEASEDPLOG 18 //Release dpLog handle (precedes Updateparams)
#define COMMAND_LOADAC   19  // Load mesh and compute AC matrix
#define COMMAND_STEPAC   20  // Perform single iteration step (AC)

static const char *prStates[] = {

        "Not started",
        "Running",
        "Waiting",
        "Killed",
        "Error",
        "Done",
        "Computing AC matrix", //Molflow only
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
        "Release dpLog",
        "Load AC matrix", //Molflow only
        "AC iteration step" //Molflow only
};

#endif //MOLFLOW_PROJ_PROCESSCONTROL_H
