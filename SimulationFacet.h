

#ifndef SYNRAD_PROJ_SIMULATIONFACET_H
#define SYNRAD_PROJ_SIMULATIONFACET_H

#include "FacetData.h"

/*
 * Generalised structure containing Ray Tracing and shared Simulation properties (mostly caches)
 * Application specific implementations are part of the corresponding code base
 */
class SimulationFacet : public RTFacet {
protected:
    SimulationFacet() = default;
    explicit SimulationFacet(size_t nbIndex);
    //SimulationFacet(const SimulationFacet& cpy);
    //SimulationFacet(SimulationFacet&& cpy) noexcept;
    
    //SimulationFacet& operator=(const SimulationFacet& cpy);
    //SimulationFacet& operator=(SimulationFacet&& cpy) noexcept;
public:
    ~SimulationFacet() = default;
    std::vector<double>   textureCellIncrements;              // Texure increment
    std::vector<bool>     largeEnough;      // cells that are NOT too small for autoscaling

    bool   isHit = false;

    void InitializeTexture();

    virtual void InitializeLinkFacet();

    [[nodiscard]] virtual size_t GetHitsSize(size_t nbMoments) const;
    [[nodiscard]] virtual size_t GetMemSize() const;

    std::vector<double> InitTextureMesh();
};



#endif //SYNRAD_PROJ_SIMULATIONFACET_H
