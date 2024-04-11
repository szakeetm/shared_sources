

#pragma once

#include <string>
#include "SMP.h"
#include "ProcessControl.h"
#include "SimulationUnit.h"
namespace MFSim {
    class ParticleTracer;
}

//class Simulation_Abstract;

enum RunResult {
    MaxReached,
    DesError,
    Success
};

enum LoopResult {
    Continue,
    AbortCommand,
    DesLimitReached,
    TimeLimitReached,
    DesorptionError,
    HasThreadError
};

/**
* \brief Inidividual simulation states and settings per thread
 * contains local desorption limits, local simulation state, global thread number, simulation state etc.
 */
class SimThreadHandle {
public:
    SimThreadHandle(ProcComm& procInfo, Simulation_Abstract* simPtr, size_t threadNum, size_t nbThreads);

    
    double stepsPerSec=1.0;
    size_t localDesLimit=0;
    double timeLimit=0.0;

    ProcComm& masterProcInfo;
    Simulation_Abstract* simulationPtr;
    size_t threadNum,nbThreads;

    std::shared_ptr<MFSim::ParticleTracer> particleTracerPtr;
    LoopResult RunLoop();
    void MarkIdle();
    [[nodiscard]] std::string ConstructMyThreadStatus() const;

private:
    
    void SetMyStatus(const std::string& msg) const;
    void SetMyState(const ThreadState state) const;
    RunResult RunSimulation1sec(const size_t desorptionLimit);
    //int advanceForTime(double simDuration);
    //int advanceForSteps(size_t desorptions);
};

/**
* \brief Listens to commands from SimulationManager and does the thread jobs, blocking.
 */
class SimulationController {
    bool UpdateParams(LoadStatus_abstract* loadStatus = nullptr);
    void ResetControls();
protected:

    //int SetThreadStates(SimState state, const std::string &status, bool changeState = true, bool changeStatus = true); //Sets for all threads the same state and status
    //int SetThreadStates(SimState state, const std::vector<std::string> &status, bool changeState = true, bool changeStatus = true);
    //std::vector<std::string> GetThreadStatuses();
    //void SetThreadError(const std::string& message);
    //void SetStatus(const std::string &status); //Sets for all
    //void SetReady(/*const bool loadOk*/);
    void ClearCommand();
    void SetRuntimeInfo();
    //size_t GetThreadStates() const;
public:
    SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                         Simulation_Abstract *simulationInstance, ProcComm& pInfo);
    void ControllerLoop();

    bool StartAndRun(LoadStatus_abstract* loadStatus=nullptr);
    void Load(LoadStatus_abstract* loadStatus = nullptr);
    void Reset(LoadStatus_abstract* loadStatus = nullptr);
    void MarkThreadsIdle(LoadStatus_abstract* loadStatus = nullptr);

    void EmergencyExit();
protected:

    Simulation_Abstract* simulationPtr;
    std::vector<SimThreadHandle> simThreadHandles;

    ProcComm& procInfo;
    size_t parentPID;
    size_t nbThreads;
    size_t prIdx;

private:
    // tmp
    double stepsPerSec=0.0;
    bool exitRequested=false;
    bool lastHitUpdateOK=true;
    bool loadOk=false;

};
