/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#ifndef MOLFLOW_PROJ_SIMULATIONMANAGER_H
#define MOLFLOW_PROJ_SIMULATIONMANAGER_H

#include <vector>
#include <string>
#include <thread>
#include "../src/Simulation/Simulation.h"
#include "ProcessControl.h"

typedef unsigned char BYTE;

class SimulationModel;
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
    NLOADERTYPES,
    LOADHITS
};

/*!
 * @brief Controls concrete Simulation instances and manages their I/O needs. Can act as a standalone (CLI mode) or as a middleman (GPU mode).
 * @todo Add logger capability to console OR sdl framework
 */
class SimulationManager {
    int CreateCPUHandle();

    int CreateGPUHandle();

    int CreateRemoteHandle();

    std::string MakeSubProcError(const char *message);

    int refreshProcStatus();
protected:

    int ForwardCommand(int command, size_t param, size_t param2);

    int WaitForProcStatus(uint8_t procStatus);

public:
    SimulationManager(int pid = -1);

    ~SimulationManager();

    int StartSimulation();

    int StopSimulation();

    int LoadSimulation();

    int ShareWithSimUnits(void *data, size_t size, LoadType loadType);

    int ExecuteAndWait(int command, uint8_t procStatus, size_t param = 0, size_t param2 = 0);

    int InitSimUnits();

    int InitSimulation(const std::shared_ptr<SimulationModel>& model, GlobalSimuState *globState);

    int KillAllSimUnits();

    int ResetSimulations();

    int ResetHits();

    int GetProcStatus(size_t *states, std::vector<std::string> &statusStrings);

    int GetProcStatus(ProcComm &procInfoList);

    std::string GetErrorDetails();

    // Hit Buffer functions
    bool GetLockedHitBuffer();

    int UnlockHitBuffer();

    bool GetRunningStatus();

    int IncreasePriority();
    int DecreasePriority();

    int RefreshRNGSeed(bool fixed);
private:
    // Direct implementation for threads
    ProcComm procInformation; // process control


protected:
    //std::vector<SimulationUnit*> simHandles; // for threaded versions
public:
    // Flags for which simulation type should be run
    bool useCPU;
    bool useGPU;
    bool useRemote;

    uint16_t nbThreads;
    uint16_t mainProcId;

    bool interactiveMode;
    bool isRunning;
    bool allProcsDone;
    bool hasErrorStatus;
    bool simulationChanged; // sendOnly functionality from Worker::RealReload

private:
    std::vector<std::pair<std::thread, SimType>> simHandles; // Vector of a pair of pid , simulation type
    //std::vector<std::thread> cpuSim;
    std::vector<SimulationController> simController;
    std::vector<Simulation*> simUnits;

public:
    void ForwardSimModel(const std::shared_ptr<SimulationModel>& model);
    void ForwardGlobalCounter(GlobalSimuState *simState, ParticleLog *particleLog);
    void ForwardOtfParams(OntheflySimulationParams* otfParams);
    void ForwardFacetHitCounts(std::vector<FacetHitBuffer*>& hitCaches);

};


#endif //MOLFLOW_PROJ_SIMULATIONMANAGER_H
