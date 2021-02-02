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
    SimThread(ProcComm* procInfo, SimulationUnit* sim, size_t threadNum);
    ~SimThread();

    size_t threadNum;
    double stepsPerSec;
    bool simEos;
    size_t localDesLimit;
    double simDuration;

    char** status;
    ProcComm* procInfo;
    SimulationUnit* simulation;
    CurrentParticleStatus* particle;
    bool runLoop();

private:
    [[nodiscard]] char *getSimStatus() const;
    void setSimState(char *msg) const;
    void setSimState(const std::string& msg) const;
    int runSimulation(size_t desorptions);
    int advanceForTime(double simDuration);
    int advanceForSteps(size_t desorptions);
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
    int SetState(size_t state, const std::vector<std::string> &status, bool changeState = true, bool changeStatus = true);
    void GetState();
    std::vector<std::string> GetSimuStatus();
    void SetErrorSub(const char *message);
    void SetStatus(char *status);
    void SetReady(const bool loadOk);
    int ClearCommand();
    int SetRuntimeInfo();
    size_t GetLocalState() const;
public:
    SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                         std::vector<SimulationUnit *> *simulationInstance, ProcComm *pInfo);
    ~SimulationController();
    SimulationController(SimulationController&& o) noexcept ;
    int controlledLoop(int argc = 0, char **argv = nullptr);

protected:

    std::vector<SimulationUnit*>* simulation; //
    std::vector<SimThread> simThreads;

    ProcComm* procInfo;
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
