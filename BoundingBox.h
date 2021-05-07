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
    AxisAlignedBoundingBox() : min(std::numeric_limits<double>::max()),
                               max(std::numeric_limits<double>::lowest())
    {}

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

    const Vector3d& operator[] (int) const;

    Vector3d Offset(const Vector3d &p) const;

    Vector3d Diagonal() const;

    int MaximumExtent() const;

    double SurfaceArea() const;

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb1, const AxisAlignedBoundingBox& bb2);

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb, const Vector3d& p);

    bool IntersectBox(const Ray &ray, const Vector3d &invDir,
                                                     const int dirIsNeg[3]) const;
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(min), CEREAL_NVP(max));
    }

} ;


#endif //MOLFLOW_PROJ_BOUNDINGBOX_H
