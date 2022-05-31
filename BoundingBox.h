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

#ifndef MOLFLOW_PROJ_BOUNDINGBOX_H
#define MOLFLOW_PROJ_BOUNDINGBOX_H

#include <cereal/cereal.hpp>
#include <RayTracing/Ray.h>
#include "Vector.h"

// profiling
#include <Helper/Chronometer.h>

namespace Profiling {
    extern CummulativeBenchmark boxStats;
    extern CummulativeBenchmark boxPStats;
}

class AxisAlignedBoundingBox{
public:
    AxisAlignedBoundingBox();

    Vector3d min;
    Vector3d max;

    void Expand(const double amount)          {
        auto& bb = *this;
        bb.min.x -= amount;   bb.min.y -= amount;   bb.min.z -= amount;
        bb.max.x += amount;   bb.max.y += amount;   bb.max.z += amount;
    }

    void Expand(const Vector3d& amount)          {
        auto& bb = *this;
        bb.min.x -= amount.x;   bb.min.y -= amount.y;   bb.min.z -= amount.z;
        bb.max.x += amount.x;   bb.max.y += amount.y;   bb.max.z += amount.z;
    }

    void Expand(const double amount, const int dim)          {
        auto& bb = *this;
        bb.min[dim] -= amount;
        bb.max[dim] += amount;
    }

    AxisAlignedBoundingBox Split(int dim, double pos, bool keepMin)          {
        auto bb = *this;
        if(keepMin)
            bb.max[dim] = pos;
        else
            bb.min[dim] = pos;
        return bb;
    }

    Vector3d& operator[] (int);
    const Vector3d& operator[] (int) const;

    Vector3d Offset(const Vector3d &p) const;

    Vector3d Diagonal() const;

    int MaximumExtent() const;

    double SurfaceArea() const;

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb1, const AxisAlignedBoundingBox& bb2);

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb, const Vector3d& p);

    bool IntersectBox(const Ray &ray, const Vector3d &invDir, const int dirIsNeg[3]) const;
    bool IntersectBox(RayStat &ray, const Vector3d &invDir, const int *dirIsNeg) const;

    bool IntersectP(const Ray &ray, double *hitt0, double *hitt1) const;
    bool IntersectP(const Ray &ray, double *hitt0, double *hitt1, int dim) const;
    bool IsInside(const Vector3d &point) const;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(min), CEREAL_NVP(max));
    }

    bool IntersectP(RayStat &ray, double *hitt0, double *hitt1) const;
} ;


#endif //MOLFLOW_PROJ_BOUNDINGBOX_H
