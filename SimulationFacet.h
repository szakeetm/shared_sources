//
// Created by Pascal Baehr on 14.06.22.
//

#ifndef SYNRAD_PROJ_SIMULATIONFACET_H
#define SYNRAD_PROJ_SIMULATIONFACET_H

#include "FacetData.h"

// Local facet structure
struct SimulationFacet : public Facet {
protected:
    SimulationFacet();
    explicit SimulationFacet(size_t nbIndex);
public:
    std::vector<double>   textureCellIncrements;              // Texure increment
    std::vector<bool>     largeEnough;      // cells that are NOT too small for autoscaling

    // Temporary var (used in FillHit for hit recording)
    bool   isReady{};         // Volatile state
    bool   isHit;
    //size_t globalId; //Global index (to identify when superstructures are present)

    // Facet hit counters
    //std::vector<FacetHitBuffer> tmpCounter; //1+nbMoment
    //std::vector<FacetHistogramBuffer> tmpHistograms; //1+nbMoment

    //void ResetCounter();
    //void ResizeCounter(size_t nbMoments);
    /*bool InitializeOnLoad(const size_t &id, const size_t &nbMoments);

    size_t InitializeHistogram(const size_t &nbMoments) const;

    size_t InitializeDirectionTexture(const size_t &nbMoments);

    size_t InitializeProfile(const size_t &nbMoments);

    size_t InitializeTexture(const size_t &nbMoments);

    size_t InitializeAngleMap();

    void InitializeOutgassingMap();

    bool InitializeLinkAndVolatile(const size_t & id);*/

    void ResetCounter();

    size_t InitializeDirectionTexture();

    size_t InitializeProfile();

    size_t InitializeTexture();

    bool InitializeLinkAndVolatile(const size_t & id);

    [[nodiscard]] virtual size_t GetHitsSize(size_t nbMoments) const;
    [[nodiscard]] size_t GetMemSize() const;

    //void RegisterTransparentPass(SubprocessFacet *facet); //Allows one shared Intersect routine between MolFlow and Synrad

    std::vector<double> InitTextureMesh();
};



#endif //SYNRAD_PROJ_SIMULATIONFACET_H
