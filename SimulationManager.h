//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONMANAGER_H
#define MOLFLOW_PROJ_SIMULATIONMANAGER_H

#include <vector>

class SimulationCore;

/*!
 * @brief Controls concrete Simulation instances and manages their I/O needs. Can act as a standalone (CLI mode) or as a middleman (GPU mode).
 */
class SimulationManager {
    int CreateCPUHandle();
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



private:
    // Flags
    bool useCPU;
    uint16_t nbCores;

    bool useGPU;

    bool useRemote;

protected:
    std::vector<SimulationCore*> simHandles;
};


#endif //MOLFLOW_PROJ_SIMULATIONMANAGER_H
