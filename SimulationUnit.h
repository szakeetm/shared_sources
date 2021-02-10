//
// Created by Pascal Baehr on 29.05.20.
//

#ifndef MOLFLOW_PROJ_SIMULATIONUNIT_H
#define MOLFLOW_PROJ_SIMULATIONUNIT_H

//#include "MolflowHitCounter.h"
#include "SMP.h"
#include "Buffer_shared.h"
#include "../src/Simulation/Particle.h"
#include <../src/GeometrySimu.h>

namespace MFSim {
    class Particle;
}

class SimulationUnit {
public:
    SimulationUnit() : model(), totalDesorbed(0), m(){
        globState = nullptr;
        globParticleLog = nullptr;
    };
    SimulationUnit(const SimulationUnit& o) : model(o.model) , m() {
        globState = nullptr;
        globParticleLog = nullptr;
        totalDesorbed = o.totalDesorbed;
    };
    virtual ~SimulationUnit()= default;

    /*! Parse input and pre compute/prepare all necessary structures  */
    virtual size_t LoadSimulation(SimulationModel *simModel, char *loadStatus) = 0;
    virtual int ReinitializeParticleLog() = 0;
    virtual int SanityCheckGeom() = 0;

    virtual void ResetSimulation() = 0;
    virtual void ClearSimulation() = 0;

    virtual size_t GetHitsSize() = 0;
    virtual MFSim::Particle * GetParticle(size_t i) = 0;
    virtual void SetNParticle(size_t n) = 0;
public:
    SimulationModel model;
    //OntheflySimulationParams ontheflyParams;
    //GeomProperties sh;
    // Particle coordinates (MC)
    GlobalSimuState* globState;
    ParticleLog* globParticleLog; //Recorded particle log since last UpdateMCHits
    //GlobalSimuState tmpResults;

    size_t totalDesorbed; // todo: should be a "sim counter"
    std::timed_mutex m;
};

#endif //MOLFLOW_PROJ_SIMULATIONUNIT_H
