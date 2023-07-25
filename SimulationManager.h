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

    int InitSimulations();

    void InitSimulation(std::shared_ptr<SimulationModel> model, GlobalSimuState *globStatePtr); //throws error

    int KillAllSimUnits();

    int ResetSimulations();

    int ResetHits();

    int GetProcStatus(size_t *states, std::vector<std::string> &statusStrings);

    int GetProcStatus(ProcComm &procInfoList);

    std::string GetErrorDetails();

    bool GetRunningStatus();

    /*
    int IncreasePriority();
    int DecreasePriority();
    */

    //int RefreshRNGSeed(bool fixed);
private:
    // Direct implementation for threads
    ProcComm procInformation; // process control, facilitates round-robin access


protected:
    //std::vector<Simulation_Abstract*> simThreads; // for threaded versions
public:
    size_t nbThreads=0;
    size_t mainProcId;

    bool interactiveMode=true; //Commands issued to threads with possible user break or timeout
    bool noProgress = false; //Don't print percentage updates for progressbars, useful if output written to log file
    bool isRunning=false;
    bool allProcsDone=false;
    bool hasErrorStatus=false;
    bool simulationChanged = true; // by default, always init simulation process the first time

private:
    std::unique_ptr<SimulationController> simController;
    std::unique_ptr<Simulation_Abstract> simulation;
    std::vector<std::thread> simThreads;

public:
    void ForwardSimModel(std::shared_ptr<SimulationModel> model);
    void ForwardGlobalCounter(GlobalSimuState *simStatePtr, ParticleLog *particleLogPtr);
    void ForwardOtfParams(OntheflySimulationParams* otfParams);
    void ForwardFacetHitCounts(std::vector<FacetHitBuffer*>& hitCaches);
};


#endif //MOLFLOW_PROJ_SIMULATIONMANAGER_H
