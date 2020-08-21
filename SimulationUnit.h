//
// Created by Pascal Baehr on 29.05.20.
//

#ifndef MOLFLOW_PROJ_SIMULATIONUNIT_H
#define MOLFLOW_PROJ_SIMULATIONUNIT_H

//#include "MolflowHitCounter.h"
#include "SMP.h"
#include "Buffer_shared.h"
#include "../src/GeometrySimu.h"

class CurrentParticleStatus;

class SimulationUnit {
public:
    virtual ~SimulationUnit()= default;;

    /*! Parse input and pre compute/prepare all necessary structures  */
    virtual size_t LoadSimulation() = 0;
    virtual void UpdateHits(Dataport *dpLog, int prIdx, DWORD timeout) = 0;
    //virtual bool UploadHits(Dataport *dpHit, Dataport* dpLog, int prIdx, DWORD timeout) = 0;

    virtual bool UpdateOntheflySimuParams(Dataport *loader) = 0;
    virtual int ReinitializeParticleLog() = 0;

    virtual int SanityCheckGeom() = 0;

    virtual bool SimulationMCStep(size_t nbStep) = 0;

    virtual void ResetSimulation() = 0;
    virtual void ClearSimulation() = 0;

    virtual size_t GetHitsSize() = 0;

public:
    SimulationModel model;
    //OntheflySimulationParams ontheflyParams;
    //GeomProperties sh;
    // Particle coordinates (MC)
    //CurrentParticleStatus* currentParticle;
    //GlobalSimuState tmpResults;

    size_t totalDesorbed; // todo: should be a "sim counter"
};

#endif //MOLFLOW_PROJ_SIMULATIONUNIT_H
