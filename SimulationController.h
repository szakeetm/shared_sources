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

class SimulationController {
    bool UpdateParams();
protected:
    int resetControls();


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
                         SimulationUnit *simulationInstance, std::shared_ptr<ProcComm> pInfo);
    ~SimulationController();
    SimulationController(SimulationController&& o) noexcept ;

    SimulationController();

    int controlledLoop(int argc = 0, char **argv = nullptr);

    virtual int Start() = 0;
    virtual bool Load() = 0;
    virtual int RebuildAccel() = 0;
    virtual int Reset() = 0;
    virtual void EmergencyExit() = 0; // Killing threads

protected:

    SimulationUnit* simulation;

    std::shared_ptr<ProcComm> procInfo;
    size_t parentPID;
    size_t nbThreads;
    int prIdx;

    // tmp
    double stepsPerSec;
    bool endState;
    bool lastHitUpdateOK;
    bool loadOk;

};

#endif //MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
