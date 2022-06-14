//
// Created by Pascal Baehr on 09.06.22.
//

#ifndef MOLFLOW_PROJ_SIMULATIONMODEL_H
#define MOLFLOW_PROJ_SIMULATIONMODEL_H

#include <map>
#include <string>
#include "RayTracing/BVH.h"

class Facet;
class AABBNODE;
class SimulationFacet;
class GlobalSimuState;

class SuperStructure {
public:
    SuperStructure() = default;
    ~SuperStructure() = default;
    //std::vector<SubprocessFacet>  facets;   // Facet handles
    std::shared_ptr<AABBNODE> aabbTree; // Structure AABB tree
    std::string strName;
    std::string strFileName;

    size_t GetMemSize(){
        size_t sum = 0;
        /*sum += sizeof (facets);
        for(auto& fac : facets)
            sum += fac.GetMemSize();*/
        sum += sizeof (aabbTree);
        return sum;
    }
};

class SimulationModel {
protected:
    SimulationModel() : otfParams(), wp(), sh(), m(), initialized(false) {};

    ~SimulationModel() = default;

    SimulationModel(SimulationModel &&o) noexcept: m(), initialized(false) {
        *this = std::move(o);
    };

    SimulationModel(const SimulationModel &o) : m(), initialized(false) {
        *this = o;
    };

public:
    virtual size_t size();

    SimulationModel &operator=(const SimulationModel &o) {
        facets = o.facets;
        structures = o.structures;

        accel.insert(accel.begin(), o.accel.begin(), o.accel.end());
        vertices3 = o.vertices3;
        otfParams = o.otfParams;
        //tdParams = o.tdParams;
        wp = o.wp;
        sh = o.sh;
        initialized = o.initialized;

        return *this;
    };

    SimulationModel &operator=(SimulationModel &&o) noexcept {
        facets = std::move(o.facets);
        structures = std::move(o.structures);

        accel = std::move(o.accel);
        vertices3 = std::move(o.vertices3);
        //tdParams = std::move(o.tdParams);
        otfParams = o.otfParams;
        wp = o.wp;
        sh = o.sh;
        initialized = o.initialized;

        return *this;
    };

    virtual int PrepareToRun() = 0;

    // Molflow will use ParameterSurfaces (for parameter outgassing) for particular construction types
    virtual int BuildAccelStructure(GlobalSimuState *globState, int accel_type, BVHAccel::SplitMethod split,
                            int bvh_width) = 0;

    int InitialiseFacets();
    void CalculateFacetParams(Facet *f);

    // Molflow only
    //void CalcTotalOutgassing();


    Surface *GetSurface(double opacity) {

        if (!surfaces.empty()) {
            auto surf = surfaces.find(opacity);
            if (surf != surfaces.end())
                return surf->second.get();
        }
        std::shared_ptr<Surface> surface;
        if (opacity == 1.0) {
            surface = std::make_shared<Surface>();
        } else if (opacity == 0.0) {
            surface = std::make_shared<TransparentSurface>();
        } else {
            surface = std::make_shared<AlphaSurface>(opacity);
        }
        surfaces.insert(std::make_pair(opacity, surface));
        return surface.get();
    };

    // Molflow model only (Time dependent)
    //Surface *GetParameterSurface(int opacity_paramId, Distribution2D *dist);

    // Sim functions
    double GetOpacityAt(SimulationFacet *f, double time) const;

    double GetStickingAt(SimulationFacet *f, double time) const;

    // Geometry Description
    std::vector<std::shared_ptr<SimulationFacet>> facets;    // All facets of this geometry

    std::vector<SuperStructure> structures;
    std::vector<Vector3d> vertices3; // Vertices (3D space)

    std::vector<std::shared_ptr<RTPrimitive>> accel;
    std::multimap<double,std::shared_ptr<Surface>> surfaces;

    // Simulation Properties
    OntheflySimulationParams otfParams;
    WorkerParams wp;

    // Geometry Properties
    GeomProperties sh;

    bool initialized;
    std::mutex m;

    void BuildPrisma(double L, double R, double angle, double s, int step);
};


#endif //MOLFLOW_PROJ_SIMULATIONMODEL_H
