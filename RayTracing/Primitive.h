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

#ifndef MOLFLOW_PROJ_RTPRIMITIVE_H
#define MOLFLOW_PROJ_RTPRIMITIVE_H

#include "BoundingBox.h"
#include <atomic>
#include <Helper/Chronometer.h>

class Ray;

struct IntersectCount{
    IntersectCount() : nbChecks(0), nbIntersects(0){};
    IntersectCount(const IntersectCount& cpy) : nbChecks(cpy.nbChecks.load()), nbIntersects(cpy.nbIntersects.load()){
        nbPrim = cpy.nbPrim;
        level = cpy.level;
    };
    IntersectCount(IntersectCount&& mvr)  noexcept : nbChecks(mvr.nbChecks.load()), nbIntersects(mvr.nbIntersects.load()){
        nbPrim = mvr.nbPrim;
        level = mvr.level;
    };
    IntersectCount& operator=(const IntersectCount& cpy){
        nbChecks = cpy.nbChecks.load();
        nbIntersects = cpy.nbIntersects.load();
        nbPrim = cpy.nbPrim;
        level = cpy.level;

        return *this;
    }

    IntersectCount& operator=(IntersectCount&& cpy) noexcept {
        nbChecks = cpy.nbChecks.load();
        nbIntersects = cpy.nbIntersects.load();
        nbPrim = cpy.nbPrim;
        level = cpy.level;

        return *this;
    }

    void Reset() {
        nbChecks = 0;
        nbIntersects = 0;
    }


    std::atomic<size_t> nbChecks{0};
    std::atomic<size_t> nbIntersects{0};
    size_t nbPrim{0};
    size_t level{0};
};

// General ray tracing interface
class RTPrimitive {
public:
    virtual ~RTPrimitive() = default;
    virtual void ComputeBB() = 0;
    virtual bool Intersect(Ray &r) = 0;
    virtual bool IntersectStat(RayStat &r) = 0;
    AxisAlignedBoundingBox bb;
};

// General acceleration data structure interface
class RTAccel : public RTPrimitive {
public:
    // Stats
    RayStatistics perRayCount;
    std::vector<IntersectCount> ints;
    std::vector<CummulativeBenchmark> algorithm_times; //time_sum,count pair for individual algorithm times
    void ResetStats(){
        perRayCount.Reset();
        for(auto& stat : ints) {
            stat.Reset();
        }
    }
    // -----
};
#endif //MOLFLOW_PROJ_RTPRIMITIVE_H
