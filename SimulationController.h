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

#pragma once

#include <string>
#include "SMP.h"
#include "ProcessControl.h"
#include "SimulationUnit.h"

//class Simulation_Abstract;

/**
* \brief Inidividual simulation states and settings per thread
 * contains local desorption limits, local simulation state, global thread number, simulation state etc.
 */
class SimHandle {
public:
    SimHandle(ProcComm* procInfoPtr, Simulation_Abstract* simPtr, size_t threadNum);

    size_t threadNum;
    double stepsPerSec;
    bool desLimitReachedOrDesError;
    size_t localDesLimit;
    double timeLimit;

    ProcComm* masterProcInfoPtr;
    Simulation_Abstract* simulationPtr;
    MFSim::ParticleTracer* particleTracerPtr;
    bool runLoop();
    [[nodiscard]] std::string ConstructThreadStatus() const;

private:
    
    void setMyStatus(const std::string& msg) const;
    bool runSimulation1sec(const size_t desorptions);
    int advanceForTime(double simDuration);
    int advanceForSteps(size_t desorptions);
};

/**
* \brief Controller that handles communication between GUI via SimulationManager and the running @Simulation_Abstract
 */
class SimulationController {
    bool UpdateParams();
    int resetControls();
protected:


    virtual int StopSim() {return 0;};
    virtual int TerminateSim() {return 0;};

    int SetThreadStates(size_t state, const std::string &status, bool changeState = true, bool changeStatus = true); //Sets for all threads the same state and status
    int SetThreadStates(size_t state, const std::vector<std::string> &status, bool changeState = true, bool changeStatus = true);
    std::vector<std::string> GetThreadStatuses();
    void SetThreadError(const std::string& message);
    void SetStatus(const std::string &status); //Sets for all
    void SetReady(const bool loadOk);
    int ClearCommand();
    int SetRuntimeInfo();
    size_t GetThreadStates() const;
public:
    SimulationController(size_t parentPID, size_t procIdx, size_t nbThreads,
                         Simulation_Abstract *simulationInstance, ProcComm *pInfo);
    //~SimulationController();
    //SimulationController(SimulationController&& o) noexcept ;
    int controlledLoop(int argc = 0, char **argv = nullptr);

    int Start();
    bool Load();
    int RebuildAccel();
    int Reset();

    void EmergencyExit();
protected:

    Simulation_Abstract* simulationPtr;
    std::vector<SimHandle> simThreadHandles;

    ProcComm* procInfoPtr;
    size_t parentPID;
    size_t nbThreads;
    size_t prIdx;

private:
    // tmp
    double stepsPerSec;
    bool endState;
    bool lastHitUpdateOK;
    bool loadOk;

};
