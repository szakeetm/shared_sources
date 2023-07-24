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

#include "SMP.h"
#include "Buffer_shared.h"

class SimulationModel;
class GlobalSimuState;
struct ParticleLog;

namespace MFSim {
    class ParticleTracer;
}

/**
* \brief Abstract Simulation unit that is implemented as Simulation for the CPU based simulations for Synrad and Molflow. Its implemented child class is Simulation. Old name is "SimulationUnit"
 */
class Simulation_Abstract {
public:
    Simulation_Abstract() = default;
    Simulation_Abstract(const Simulation_Abstract& o) : model(o.model)/* , modelMutex()*/ {
        globStatePtr = nullptr;
        globParticleLogPtr = nullptr;
        totalDesorbed = o.totalDesorbed;
    };
    virtual ~Simulation_Abstract()= default;

    /*! Parse input and pre compute/prepare all necessary structures  */
    virtual size_t LoadSimulation(std::string& loadStatus) = 0;
    virtual int RebuildAccelStructure() = 0;

    virtual int ReinitializeParticleLog() = 0;
    virtual std::pair<int, std::optional<std::string>> SanityCheckModel(bool strictCheck) = 0;

    virtual void ResetSimulation() = 0;
    virtual void ClearSimulation() = 0;

    virtual size_t GetHitsSize() = 0;
    virtual MFSim::ParticleTracer * GetParticleTracerPtr(size_t i) = 0;
    virtual void ConstructParticleTracers(size_t n, bool fixedSeed) = 0;
public:
    std::shared_ptr<SimulationModel> model;
    GlobalSimuState* globStatePtr=nullptr;
    ParticleLog* globParticleLogPtr=nullptr; //Recorded particle log since last UpdateMCHits

    size_t totalDesorbed=0; // todo: should be a "sim counter"
};
