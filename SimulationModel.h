

#pragma once
#include "RayTracing/BVH.h"
#include "SimulationFacet.h"
#include "IntersectAABB_shared.h"
#include "Buffer_shared.h"

#include <map>
#include <string>
#include <mutex>

class RTFacet;
class GlobalSimuState;

class SuperStructure {
public:
    std::string name;

    size_t GetMemSize(){
        size_t sum = sizeof(name);
        return sum;
    }
};

/*
 * Generalised structure containing all the data that is necessary for a simulation
 * Geometric properties and various simulation settings
 * Application specific implementations are part of the corresponding code base
 */
class SimulationModel {
protected:
    /*
    SimulationModel() = default;

    ~SimulationModel() = default;

    
    SimulationModel(SimulationModel &&o) noexcept {
        *this = std::move(o);
    };
    
    SimulationModel(const SimulationModel &o) {
        *this = o;
    };
    */

public:
    virtual size_t GetMemSize();
    /*
    SimulationModel &operator=(const SimulationModel &o) {
        facets = o.facets;
        structures = o.structures;

        rayTracingStructures.insert(rayTracingStructures.begin(), o.rayTracingStructures.begin(), o.rayTracingStructures.end());
        vertices3 = o.vertices3;
        otfParams = o.otfParams;
        //tdParams = o.tdParams;
        sp = o.sp;
        sh = o.sh;
        initialized = o.initialized;

        return *this;
    };

    SimulationModel &operator=(SimulationModel &&o) noexcept {
        facets = std::move(o.facets);
        structures = std::move(o.structures);

        rayTracingStructures = std::move(o.rayTracingStructures);
        vertices3 = std::move(o.vertices3);
        //tdParams = std::move(o.tdParams);
        otfParams = o.otfParams;
        sp = o.sp;
        sh = o.sh;
        initialized = o.initialized;

        return *this;
    };
    */
    virtual void PrepareToRun() = 0; //throws error
    virtual std::vector<std::string> SanityCheck()=0;
    // Molflow will use ParameterSurfaces (for parameter outgassing) for particular construction types
    virtual int BuildAccelStructure(const std::shared_ptr<GlobalSimuState> globalState, AccelType accel_type, BVHAccel::SplitMethod split,
                            int bvh_width) = 0;

    int InitializeFacets();
    void CalculateFacetParams(RTFacet *f);

    // Molflow only
    //void CalcTotalOutgassing();

    virtual std::shared_ptr<Surface> GetSurface(std::shared_ptr<SimulationFacet> facet) {
        double opacity = facet->sh.opacity;
        if (!surfaces.empty()) {
            auto surf = surfaces.find(opacity);
            if (surf != surfaces.end())
                return surf->second;
        }
        //not found, construct new
        std::shared_ptr<Surface> surface;
        if (opacity == 1.0) {
            surface = std::make_shared<Surface>();
        } else if (opacity == 0.0) {
            surface = std::make_shared<TransparentSurface>();
        } else {
            surface = std::make_shared<SemiTransparentSurface>(opacity);
        }
        surfaces.insert(std::make_pair(opacity, surface));
        return surface;
    };

    // Sim functions
    virtual double GetOpacityAt(SimulationFacet *f, double time) const {return -1.0;};
    virtual double GetStickingAt(SimulationFacet *f, double time) const {return -1.0;};

    size_t memSizeCache=0;

    // Geometry Description
    std::vector<std::shared_ptr<SimulationFacet>> facets; // All facets of this sim model. Shared pointer as these facets are referred to in the ray tracing accel structure

    std::vector<SuperStructure> structures;
    std::vector<Vector3d> vertices3; // Vertices (3D space)

    std::vector<std::unique_ptr<RTPrimitive>> rayTracingStructures; //One raytracing rayTracingStructures. model per superstructure
    std::map<double,std::shared_ptr<Surface>> surfaces; //Pair of opacity -> facet surface type

    // Simulation Properties
    OntheflySimulationParams otfParams;
    SimuParams sp;

    // Geometry Properties
    GeomProperties sh;

    bool initialized = false; //set true if PrepareToRun() succeeds (parameter id match, outgassing calc, sanity check etc)
    std::mutex modelMutex;

    //virtual void BuildPrisma(double L, double R, double angle, double s, int step) {};
};