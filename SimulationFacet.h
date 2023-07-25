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
    
    SimulationFacet& operator=(const SimulationFacet& cpy);
    SimulationFacet& operator=(SimulationFacet&& cpy) noexcept;
public:
    ~SimulationFacet() = default;
    std::vector<double>   textureCellIncrements;              // Texure increment
    std::vector<bool>     largeEnough;      // cells that are NOT too small for autoscaling

    // Temporary var (used in FillHit for hit recording)
    bool   isReady = false;         // Volatile state
    bool   isHit = false;

    void InitializeTexture();

    virtual bool InitializeLinkAndVolatile(const size_t  id);

    [[nodiscard]] virtual size_t GetHitsSize(size_t nbMoments) const;
    [[nodiscard]] virtual size_t GetMemSize() const;

    std::vector<double> InitTextureMesh();
};



#endif //SYNRAD_PROJ_SIMULATIONFACET_H
