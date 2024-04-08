

#pragma once

#include <cereal/cereal.hpp>
#include <limits>
#include "Vector.h"

class AxisAlignedBoundingBox{
public:
    AxisAlignedBoundingBox();

    Vector3d min;
    Vector3d max;

    Vector3d& operator[] (int);
    const Vector3d& operator[] (int) const;

    Vector3d Offset(const Vector3d &p) const;

    Vector3d Diagonal() const;

    int MaximumExtent() const;

    double SurfaceArea() const;

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb1, const AxisAlignedBoundingBox& bb2);

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb, const Vector3d& p);

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(min), CEREAL_NVP(max));
    }

} ;

//! Epsilon value for error threshold
constexpr double machEps = std::numeric_limits<double>::epsilon() * 0.5;

//! Value based on error threshold to define robust intersection bounds
constexpr double gamma(int n)
{
    return (n * machEps) / (1 - n * machEps);
}