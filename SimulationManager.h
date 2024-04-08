

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

    int RefreshProcStatus();
protected:

    void ForwardCommand(SimCommand command, size_t param, size_t param2);

    int WaitForControllerAndThreadState(const std::vector<ControllerState>& successControllerStates, const std::vector<ThreadState>& successThreadStates,
        LoadStatus_abstract* loadStatus =nullptr);

public:
    SimulationManager(int pid = -1);

    ~SimulationManager();

    void StartSimulation(LoadStatus_abstract* loadStatus = nullptr);

    void StopSimulation(LoadStatus_abstract* loadStatus = nullptr);

    void LoadSimulation(LoadStatus_abstract* loadStatus = nullptr);

    void ShareWithSimUnits(void *data, size_t size, LoadType loadType, LoadStatus_abstract* loadStatus = nullptr);

    int ExecuteAndWait(const SimCommand command, const size_t param, const size_t param2,
        const std::vector<ControllerState>& successControllerStates, const std::vector<ThreadState>& successThreadStates,
        LoadStatus_abstract* loadStatus);

    //void UpdateLimitReachedAndErrorStates();

    int SetUpSimulation(LoadStatus_abstract* loadStatus=nullptr);

    void InitSimulation(std::shared_ptr<SimulationModel> model, const std::shared_ptr<GlobalSimuState> globalState); //throws error

    void KillSimulation(LoadStatus_abstract* loadStatus=nullptr);

    void ResetSimulations(LoadStatus_abstract* loadStatus=nullptr);

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
    bool allProcsReachedLimit=false;
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

