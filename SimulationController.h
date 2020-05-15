//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
#define MOLFLOW_PROJ_SIMULATIONCONTROLLER_H

//#include "Simulation.h"
#include <string>
#include "Buffer_shared.h"
#include "SMP.h"
#include "ProcessControl.h"

class CurrentParticleStatus;
class MCSimulation {
public:
    OntheflySimulationParams ontheflyParams;
    GeomProperties sh;
    // Particle coordinates (MC)
    CurrentParticleStatus* currentParticle;
};

class SimulationController {
    virtual bool Load() = 0;
public:
    // tmp
    bool loadOK;
protected:

    /*! Parse input and pre compute/prepare all necessary structures  */
    virtual bool LoadSimulation(Dataport *loader) = 0;
    virtual bool UpdateParams() = 0;

    int StartSimulation();
    virtual int SanityCheckGeom() = 0;

    int RunSimulation();
    virtual bool SimulationMCStep(size_t nbStep) = 0;
    virtual int StopSim() {};
    virtual int TerminateSim() {};

    virtual void ResetSimulation() = 0;
    virtual void ClearSimulation() = 0;

    virtual void UpdateHits(Dataport *dpHit, Dataport* dpLog,int prIdx, DWORD timeout) = 0;

    int SetState(size_t state, const char *status, bool changeState = true, bool changeStatus = true);
    void GetState();
    char *GetSimuStatus();
    void SetErrorSub(const char *message);
    void SetStatus(char *status);
    void SetReady();
    size_t GetLocalState() const;
public:
    SimulationController(std::string appName , std::string dpName, size_t parentPID, size_t procIdx);
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

protected:
    OntheflySimulationParams ontheflyParams;
    GeomProperties sh;
    // Particle coordinates (MC)
    CurrentParticleStatus* currentParticle;

    double stepsPerSec;
    size_t totalDesorbed; // todo: should be a "sim counter"
    int prIdx;
    size_t parentPID;
    SubProcInfo procInfo;
    bool endState;
    bool lastHitUpdateOK;
};

#endif //MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
