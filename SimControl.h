//
// Created by pbahr on 4/5/22.
//

#ifndef MOLFLOW_PROJ_SIMCONTROL_H
#define MOLFLOW_PROJ_SIMCONTROL_H

class SimControl {
    bool UpdateParams();
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
    SimulationUnit *simulationInstance, ProcComm *pInfo);
    ~SimulationController();
    SimulationController(SimulationController&& o) noexcept ;
    int controlledLoop(int argc = 0, char **argv = nullptr);

    int Start();
    bool Load();
    int RebuildAccel();
    int Reset();

    void EmergencyExit(){
        for(auto& t : simThreads)
            t.particle->allQuit = true;
    };
protected:

    SimulationUnit* simulation;
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
    bool loadOk;
};

#endif //MOLFLOW_PROJ_SIMCONTROL_H
