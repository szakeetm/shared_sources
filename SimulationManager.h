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

#pragma once

#include <vector>
#include <string>
#include <thread>
#ifdef MOLFLOW
#include "../src/Simulation/MolflowSimulation.h"
#endif

#ifdef SYNRAD
#include "../src/Simulation/SynradSimulation.h"
#endif
#include "ProcessControl.h"

typedef unsigned char BYTE;

class SimulationModel;
class SimulationController;

struct Dataport;

/*
enum class SimType : uint8_t {
    simCPU,
    simGPU,
    simRemote
};
*/

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
    int CreateCPUHandle(LoadStatus_abstract* loadStatus = nullptr);

    std::string MakeSubProcError(const std::string& message);

    int refreshProcStatus();
protected:

    void ForwardCommand(SimCommand command, size_t param, size_t param2);

    int WaitForControllerAndThreadState(const std::optional<ControllerState>& successControllerState, const std::optional<ThreadState>& successThreadState,
        LoadStatus_abstract* loadStatus =nullptr);

public:
    SimulationManager(int pid = -1);

    ~SimulationManager();

    int StartSimulation(LoadStatus_abstract* loadStatus = nullptr);

    int StopSimulation(LoadStatus_abstract* loadStatus = nullptr);

    int LoadSimulation(LoadStatus_abstract* loadStatus = nullptr);

    int ShareWithSimUnits(void *data, size_t size, LoadType loadType, LoadStatus_abstract* loadStatus = nullptr);

    int ExecuteAndWait(const SimCommand command, const size_t param, const size_t param2,
        const std::optional<ControllerState>& successControllerStateconst, const std::optional<ThreadState>& successThreadState,
        LoadStatus_abstract* loadStatus);

    int SetUpSimulation(LoadStatus_abstract* loadStatus=nullptr);

    void InitSimulation(std::shared_ptr<SimulationModel> model, const std::shared_ptr<GlobalSimuState> globalState); //throws error

    int KillSimulation(LoadStatus_abstract* loadStatus=nullptr);

    int ResetSimulations();

    //int ResetHits();

    int GetProcStatus(size_t *states, std::vector<std::string> &statusStrings);

    int GetProcStatus(ProcComm &procInfoList);

    std::string GetControllerStatus();

    std::string GetErrorDetails();

    bool IsRunning();

    /*
    int IncreasePriority();
    int DecreasePriority();
    */

    //int RefreshRNGSeed(bool fixed);
private:
    // Direct implementation for threads
    ProcComm procInformation; // process control, facilitates round-robin access


protected:
    //std::vector<Simulation_Abstract*> controllerLoopThread; // for threaded versions
public:
    size_t nbThreads=0;
    size_t mainProcId;

    bool asyncMode=false; //Commands issued to threads with non-blocking mode. Default for GUI, disabled for CLI and test suite
    bool noProgress = false; //Don't print percentage updates for progressbars, useful if output written to log file
    bool isRunning=false;
    bool allProcsFinished=false;
    bool hasErrorStatus=false;
    bool simulationChanged = true; // by default, always init simulation process the first time

private:
    std::unique_ptr<SimulationController> simController; //One day there can be several parallel simulations, so keep it separated from simulationManager
    std::unique_ptr<Simulation_Abstract> simulation;
    std::unique_ptr<std::thread> controllerLoopThread; //pointer so it can have unitialized (nullptr) state (as opposed to std::thread)

public:
    void ShareSimModel(std::shared_ptr<SimulationModel> model);
    void ShareGlobalCounter(const std::shared_ptr<GlobalSimuState> globalState, const std::shared_ptr<ParticleLog> particleLog); //Let simManager be aware of externally constructed sim state
    void SetOntheflyParams(OntheflySimulationParams* otfParams);
    void SetFacetHitCounts(std::vector<FacetHitBuffer*>& hitCaches); //facet counters part of global counter. Only for moment 0.
};

