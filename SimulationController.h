/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#ifndef MOLFLOW_PROJ_SIMULATIONCONTROLLER_H
#define MOLFLOW_PROJ_SIMULATIONCONTROLLER_H

#include <string>
#include "SMP.h"
#include "ProcessControl.h"
#include "SimulationUnit.h"

class Simulation;

/**
* \brief Controller that handles communication between GUI via SimulationManager and the running @SimulationUnit
 */
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
    SimulationController() = default;
    SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                         SimulationUnit *simulationInstance, std::shared_ptr<ProcComm> pInfo);
    ~SimulationController();
    SimulationController(SimulationController&& o) noexcept ;

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
