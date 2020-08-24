//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONMANAGER_H
#define MOLFLOW_PROJ_SIMULATIONMANAGER_H

#include <vector>
#include <string>
#include <thread>
#include "../src/Simulation.h"
#include "../src/GeometrySimu.h"
#include "ProcessControl.h"

typedef unsigned char BYTE;

class SimulationController;

struct Dataport;

enum class SimType : uint8_t {
    simCPU,
    simGPU,
    simRemote
};

enum class LoadType : uint8_t {
    LOADGEOM,
    LOADPARAM,
    LOADAC,
    NLOADERTYPES,
    LOADHITS
};

/*struct SubProcInfo {
    size_t procId;
    size_t masterCmd;
    size_t cmdParam;
    size_t cmdParam2;
    size_t oldState;
    char statusString[128];
};*/

/*!
 * @brief Controls concrete Simulation instances and manages their I/O needs. Can act as a standalone (CLI mode) or as a middleman (GPU mode).
 * @todo Add logger capability to console OR sdl framework
 */
class SimulationManager {
    int CreateCPUHandle(uint16_t iProc);

    int CreateGPUHandle();

    int CreateRemoteHandle();

    std::string MakeSubProcError(const char *message);

    int refreshProcStatus();
protected:
    /*! Load/Forward serialized simulation data (pre-processed geometry data) */
    int ResetStatsAndHits(); /*! Reset local and global stats and counters */
    int TerminateSimHandles();

    /*! Get results from simulations and clear local counters */
    bool StartStopSimulation();

    // Open/Close for shared memory
    int CreateLoaderDP(size_t loaderSize);

    int CloseLoaderDP();

    int CreateControlDP();

    int CloseControlDP();

    int CreateLogDP(size_t logDpSize);

    int CloseLogDP();

    int CreateHitsDP(size_t hitSize);

    int CloseHitsDP();

    int ForwardCommand(int command, size_t param, size_t param2);

    int WaitForProcStatus(uint8_t procStatus);

    // Load Buffer functions
    int UploadToLoader(void *data, size_t size);

    int UploadToHitBuffer(void *data, size_t size);

public:
    SimulationManager(std::string appName, std::string dpName);

    ~SimulationManager();

    int StartSimulation();

    int StopSimulation();

    int ShareWithSimUnits(void *data, size_t size, LoadType loadType);

    int ReloadLogBuffer(size_t logSize, bool ignoreSubs); /*! Reload the logger if necessary */
    int ReloadHitBuffer(size_t hitSize); /*! Reload the hits buffer if necessary */

    int ExecuteAndWait(int command, uint8_t procStatus, size_t param = 0, size_t param2 = 0);

    int InitSimUnits();

    int KillAllSimUnits();

    int ResetSimulations();

    int ResetHits();

    int ClearLogBuffer();

    int GetProcStatus(size_t *states, std::vector<std::string> &statusStrings);

    int GetProcStatus(std::vector<SubProcInfo> &procInfoList);

    const char *GetErrorDetails();

    // Hit Buffer functions
    BYTE *GetLockedHitBuffer();

    int UnlockHitBuffer();

    // Log Buffer functions
    BYTE *GetLockedLogBuffer();

    int UnlockLogBuffer();


    int LoadInput(const std::string& fileName);

private:
    bool isRunning;

    // Dataport handles and names
    //SubProcInfo procInformation[MAX_PROCESS];
    Dataport *dpHit; //TODO: Size unknown if not transferred via ReloadHitBuffer()/ShareWithSimUnits()
    Dataport *dpLog;

    // Direct implementation for threads
    std::vector<SubProcInfo> procInformation; // ctrl
    // SimulationModel* model; // load
    // hits


protected:
    char appName[16]{};
    char ctrlDpName[32]{};
    char loadDpName[32]{};
    char hitsDpName[32]{};
    char logDpName[32]{};
    //std::vector<SimulationUnit*> simHandles; // for threaded versions
public:
    // Flags
    bool useCPU;
    bool useGPU;
    bool useRemote;

    uint16_t nbCores;
    uint16_t nbThreads;
    uint16_t mainProcId{};

    bool allProcsDone;
    bool simulationChanged{}; // sendOnly functionality from Worker::RealReload
    std::vector<std::pair<std::thread, SimType>> simHandles; // Vector of a pair of pid , simulation type
    //std::vector<std::thread> cpuSim;
    std::vector<SimulationController> simController;
    std::vector<Simulation> simUnits;

};


#endif //MOLFLOW_PROJ_SIMULATIONMANAGER_H
