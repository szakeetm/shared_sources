//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
#define MOLFLOW_PROJ_SIMULATIONCONTROLLER_H

#include <string>
#include "SMP.h"
#include "ProcessControl.h"
#include "SimulationUnit.h"

class Simulation;

class SimThread {
public:
    SimThread(size_t* state, size_t* master, SimulationUnit* simu);
    ~SimThread();

    size_t* slaveState;
    size_t* masterState;
    char** status;
    double stepsPerSec;
    SimulationUnit* simulation;

    bool runLoop(size_t threadNum);
    [[nodiscard]] char *getSimuStatus() const;
    int runSimulation(size_t threadNum);
};

class SimulationController {
    bool Load();
    bool UpdateParams();
    int StartSimulation();
    int RunSimulation();
    int resetControls();

protected:


    virtual int StopSim() {return 0;};
    virtual int TerminateSim() {return 0;};

    int SetState(size_t state, const char *status, bool changeState = true, bool changeStatus = true);
    void GetState();
    char *GetSimuStatus();
    void SetErrorSub(const char *message);
    void SetStatus(char *status);
    void SetReady(const bool loadOk);
    int ClearCommand();
    int SetRuntimeInfo();
    size_t GetLocalState() const;
public:
    SimulationController(const std::string &appName, size_t parentPID, size_t procIdx, size_t nbThreads,
                         SimulationUnit *simulationInstance, SubProcInfo *pInfo);
    ~SimulationController();
    SimulationController(SimulationController&& o) noexcept ;
    int controlledLoop(int argc = 0, char **argv = nullptr);

protected:
    char appName[16];

    SimulationUnit* simulation; //
    SubProcInfo* procInfo;
    size_t parentPID;
    size_t nbThreads;
    int prIdx;

private:
    // tmp
    double stepsPerSec;
    bool endState;
    bool lastHitUpdateOK;
    bool loadOK;

};

#endif //MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
