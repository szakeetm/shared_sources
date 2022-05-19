//
// Created by pbahr on 15/04/2020.
//

#ifndef MOLFLOW_PROJ_SIMULATIONCONTROLLERCPU_H
#define MOLFLOW_PROJ_SIMULATIONCONTROLLERCPU_H

#include <string>
#include "SMP.h"
#include "ProcessControl.h"
#include "SimulationUnit.h"
#include "SimulationController.h"

class Simulation;

class SimThread {
public:
    SimThread(ProcComm* procInfo, SimulationUnit* sim, size_t threadNum);
    ~SimThread();

    size_t threadNum;
    double stepsPerSec;
    bool simEos;
    size_t localDesLimit;
    double timeLimit;

    char** status;
    ProcComm* procInfo;
    SimulationUnit* simulation;
    MFSim::Particle* particle;
    bool runLoop();

private:
    [[nodiscard]] char *getSimStatus() const;
    void setSimState(char *msg) const;
    void setSimState(const std::string& msg) const;
    int runSimulation(size_t desorptions);
    int advanceForTime(double simDuration);
    int advanceForSteps(size_t desorptions);
};

class SimulationControllerCPU : public SimulationController{
    bool UpdateParams();
    int resetControls();
protected:


    virtual int StopSim() {return 0;};
    virtual int TerminateSim() {return 0;};

public:
    SimulationControllerCPU(size_t parentPID, size_t procIdx, size_t nbThreads,
                            SimulationUnit *simulationInstance, std::shared_ptr<ProcComm> pInfo);
    ~SimulationControllerCPU();
    SimulationControllerCPU(SimulationControllerCPU&& o) noexcept ;

    int Start();
    bool Load();
    int RebuildAccel();
    int Reset();

    void EmergencyExit(){
        for(auto& t : simThreads)
            t.particle->allQuit = true;
    };
protected:

    std::vector<SimThread> simThreads;

/*private:
    // tmp
    double stepsPerSec;
    bool endState;
    bool lastHitUpdateOK;
    bool loadOk;*/

};

#endif //MOLFLOW_PROJ_SIMULATIONCONTROLLERCPU_H
