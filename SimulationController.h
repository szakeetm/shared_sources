//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
#define MOLFLOW_PROJ_SIMULATIONCONTROLLER_H

//#include "Simulation.h"
#include <string>
#include "SMP.h"
#include "ProcessControl.h"
#include "SimulationUnit.h"

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
    void SetReady();
    int ClearCommand();
    int SetRuntimeInfo();
    size_t GetLocalState() const;
public:
    SimulationController(std::string appName , std::string dpName, size_t parentPID, size_t procIdx, SimulationUnit *simulationInstance);
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

    SimulationUnit* simulation;
protected:

    int prIdx;
    size_t parentPID;
    SubProcInfo procInfo;


private:
    // tmp
    double stepsPerSec;
    bool endState;
    bool lastHitUpdateOK;
    bool loadOK;

};

#endif //MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
