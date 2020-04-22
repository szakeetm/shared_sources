//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONMANAGER_H
#define MOLFLOW_PROJ_SIMULATIONMANAGER_H

#include <vector>

typedef unsigned char BYTE;

//#include "SMP.h"
//#include "Buffer_shared.h" // TODO: Move process control defines out

class SimulationCore;
class Dataport;

enum class SimType : uint8_t{
    simCPU,
    simGPU,
    simRemote
};

struct ProcInfo{
    size_t procId;
    size_t statusId;
    size_t cmdParam;
    size_t cmdParam2;
    std::string statusString;
};

/*!
 * @brief Controls concrete Simulation instances and manages their I/O needs. Can act as a standalone (CLI mode) or as a middleman (GPU mode).
 * @todo Add logger capability to console OR sdl framework
 */
class SimulationManager {
    int CreateCPUHandle(uint16_t iProc);
    int CreateGPUHandle();
    int CreateRemoteHandle();
protected:
    int LoadInput(); /*! Load/Forward serialized simulation data (pre-processed geometry data) */
    int ResetStatsAndHits(); /*! Reset local and global stats and counters */
    int ChangeSimuParams(); /*! Load changes to simulation parameters */
    int StartSimulation();
    int StopSimulation();
    int TerminateSimHandles();
    int FetchResults(); /*! Get results from simulations and clear local counters */


public:
    SimulationManager();
    ~SimulationManager();

    int ForwardCommand(int command, size_t param = 0) const;
    int WaitForProcStatus(uint8_t procStatus);
    int ExecuteAndWait(int command, uint8_t procStatus, size_t param = 0);

    int InitSimUnits();
    int KillAllSimUnits();
    int CreateLoaderDP(size_t loaderSize);
    int CreateControlDP();
    int CreateLogDP(size_t logDpSize);
    int CreateHitsDP(size_t hitSize);

    int CloseLoaderDP();
    int CloseControlDP();
    int CloseLogDP();
    int CloseHitsDP();

    int ClearHitsBuffer();
    int GetProcStatus(size_t *states, std::vector<std::string>& statusStrings);
    int GetProcStatus(std::vector<ProcInfo>& procInfoList);

    // Hit Buffer functions
    BYTE* GetLockedHitBuffer();
    int UnlockHitBuffer();
    int UploadToHitBuffer(void *data, size_t size);

    // Load Buffer functions
    int UploadToLoader(void* data, size_t size);

    // Log Buffer functions
    BYTE* GetLockedLogBuffer();
    int UnlockLogBuffer();

    // Flags
    bool useCPU;

    bool useGPU;

    bool useRemote;
    uint16_t nbCores;
    uint16_t mainProcId;

    bool allProcsDone;

    std::vector<std::pair<uint32_t,SimType>> simHandles; // Vector of a pair of pid , simulation type

private:


    // Dataport handles and names
    Dataport *dpControl;
    Dataport *dpHit;
    size_t hitSize;
    Dataport *dpLog;
    Dataport *dpLoader;

    char appName[16];
    char      ctrlDpName[32];
    char      loadDpName[32];
    char      hitsDpName[32];
    char      logDpName[32];


protected:
    //std::vector<SimulationCore*> simHandles; // for threaded versions
};


#endif //MOLFLOW_PROJ_SIMULATIONMANAGER_H
