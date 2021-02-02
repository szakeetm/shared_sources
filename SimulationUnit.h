//
// Created by Pascal Baehr on 29.05.20.
//

#ifndef MOLFLOW_PROJ_SIMULATIONUNIT_H
#define MOLFLOW_PROJ_SIMULATIONUNIT_H

//#include "MolflowHitCounter.h"
#include "SMP.h"
#include "Buffer_shared.h"
#include <../src/GeometrySimu.h>

class CurrentParticleStatus;

struct SubProcessFacetTempVar {
    // Temporary var (used in Intersect for collision)
    SubProcessFacetTempVar(){
        colDistTranspPass=1.0E99;
        colU = 0.0;
        colV = 0.0;
        isHit=false;
    }
    double colDistTranspPass;
    double colU;
    double colV;
    bool   isHit;
};

class SimulationUnit {
public:
    SimulationUnit() : model(), totalDesorbed(0), m(){
        globState = nullptr;
    };
    SimulationUnit(const SimulationUnit& o) : model(o.model) , m() {
        globState = nullptr;
        totalDesorbed = o.totalDesorbed;
    };
    virtual ~SimulationUnit()= default;

    /*! Parse input and pre compute/prepare all necessary structures  */
    virtual size_t LoadSimulation(SimulationModel *simModel, char *loadStatus) = 0;
    //virtual bool UpdateHits(int prIdx, DWORD timeout) = 0;

    //virtual bool UpdateOntheflySimuParams(Dataport *loader) = 0;
    virtual int ReinitializeParticleLog() = 0;

    virtual int SanityCheckGeom() = 0;

    //virtual bool SimulationMCStep(size_t nbStep, size_t threadNum) = 0;

    virtual void ResetSimulation() = 0;
    virtual void ClearSimulation() = 0;

    virtual size_t GetHitsSize() = 0;
    virtual CurrentParticleStatus* GetParticle() = 0;
public:
    SimulationModel model;
    //OntheflySimulationParams ontheflyParams;
    //GeomProperties sh;
    // Particle coordinates (MC)
    GlobalSimuState* globState;
    //GlobalSimuState tmpResults;

    size_t totalDesorbed; // todo: should be a "sim counter"
    std::timed_mutex m;
};

#endif //MOLFLOW_PROJ_SIMULATIONUNIT_H
