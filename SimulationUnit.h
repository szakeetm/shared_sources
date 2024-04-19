

#pragma once

#include "SMP.h"
#include "Buffer_shared.h"
#include <vector>
#include <string>

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
    virtual ~Simulation_Abstract()= default;

    /*! Parse input and pre compute/prepare all necessary structures  */
    virtual size_t LoadSimulation(ProcCommData& procInfo, LoadStatus_abstract* loadStatus) = 0;
    virtual int RebuildAccelStructure() = 0;

    virtual int ReinitializeParticleLog() = 0;
    virtual std::vector<std::string> SanityCheckModel(bool strictCheck) = 0;

    virtual void ResetSimulation() = 0;
    //virtual void ClearSimulation() = 0;

    virtual size_t GetHitsSize() = 0;
    virtual std::shared_ptr<MFSim::ParticleTracer> GetParticleTracerPtr(size_t i) = 0;
    virtual void ConstructParticleTracers(size_t n, bool fixedSeed) = 0;
public:
    std::shared_ptr<SimulationModel> model; //constructed outside, shared
    std::shared_ptr<GlobalSimuState> globalState; //Set by SimManager->SetGlobalCounters(), constructed by Worker or CLI
    std::shared_ptr<ParticleLog> globParticleLog; //Recorded particle log since last UpdateMCHits. Set by SimManager->SetGlobalCounters(), constructed by Worker or CLI

};
