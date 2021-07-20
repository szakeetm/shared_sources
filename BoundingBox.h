//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_BOUNDINGBOX_H
#define MOLFLOW_PROJ_BOUNDINGBOX_H

#include <cereal/cereal.hpp>
#include <RayTracing/Ray.h>
#include "Vector.h"

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

    bool IntersectBox(const Ray &ray, const Vector3d &invDir,
                                                     const int dirIsNeg[3]) const;
    bool IntersectP(const Ray &ray, double *hitt0,
                                            double *hitt1) const;
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(min), CEREAL_NVP(max));
    }

} ;


#endif //MOLFLOW_PROJ_BOUNDINGBOX_H
