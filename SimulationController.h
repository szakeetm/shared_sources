//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
#define MOLFLOW_PROJ_SIMULATIONCONTROLLER_H

#include <string>
#include "SMP.h"
#include "ProcessControl.h"

class Simulation;
class SimulationController {
    bool Load();
    bool UpdateParams();
    int StartSimulation();
    int RunSimulation();

protected:


    virtual int StopSim() {return 0;};
    virtual int TerminateSim() {return 0;};

    int SetState(size_t state, const char *status, bool changeState = true, bool changeStatus = true);
    void GetState();
    char *GetSimuStatus();
    void SetErrorSub(const char *message);
    void SetStatus(char *status);
    void SetReady();
    int ClearCommand();
    int SetRuntimeInfo();
    size_t GetLocalState() const;
public:
    SimulationController(std::string appName , std::string dpName, size_t parentPID, size_t procIdx, Simulation *simulationInstance);
    ~SimulationController();
    int controlledLoop(int argc = 0, char **argv = nullptr);

protected:
    char appName[16];
    char ctrlDpName[32];
    char loadDpName[32];
    char hitsDpName[32];
    char logDpName[32];

    Dataport *dpControl;
    Dataport *dpHit;
    Dataport *dpLog;

    Simulation* simulation; //
protected:

    double stepsPerSec;

    int prIdx;
    size_t parentPID;
    SubProcInfo procInfo;
    bool endState;
    bool lastHitUpdateOK;

public:
    // tmp
    bool loadOK;
};

#endif //MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
