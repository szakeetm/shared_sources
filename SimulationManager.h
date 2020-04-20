//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONMANAGER_H
#define MOLFLOW_PROJ_SIMULATIONMANAGER_H

#include <vector>
#include "SMP.h"
#include "Buffer_shared.h" // TODO: Move process control defines out

class SimulationCore;

enum class SimType : uint8_t{
    simCPU,
    simGPU,
    simRemote
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

    int ForwardCommand(int command, size_t param = 0);
    int WaitForProcStatus(const uint8_t procStatus);

    int InitSimUnits();
    int KillAllSimUnits();
    int CreateLoaderDP(std::string loaderString);
    int CreateControlDP();
    int CreateLogDP(size_t logDpSize);
    int CreateHitsDP(size_t hitSize);

    int CloseLoaderDP();
    int CloseControlDP();
    int CloseLogDP();
    int CloseHitsDP();

    int ClearHitsBuffer();
    int GetProcStatus(size_t *states, std::vector<std::string>& statusStrings);

    // Flags
    bool useCPU;

    bool useGPU;

    bool useRemote;
    uint16_t nbCores;

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

    bool allProcsDone;

protected:
    //std::vector<SimulationCore*> simHandles; // for threaded versions
};


#endif //MOLFLOW_PROJ_SIMULATIONMANAGER_H
