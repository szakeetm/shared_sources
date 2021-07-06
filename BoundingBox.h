//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_BOUNDINGBOX_H
#define MOLFLOW_PROJ_BOUNDINGBOX_H

#include <cereal/cereal.hpp>
#include <RayTracing/Ray.h>
#include "Vector.h"

using FLOAT = float;

class AxisAlignedBoundingBox{
public:
    AxisAlignedBoundingBox();

    Vector3_t<FLOAT> min;
    Vector3_t<FLOAT> max;

    Vector3_t<FLOAT>& operator[] (int);
    const Vector3_t<FLOAT>& operator[] (int) const;

    Vector3_t<FLOAT> Offset(const Vector3_t<FLOAT> &p) const;

    Vector3_t<FLOAT> Diagonal() const;

    int MaximumExtent() const;

    double SurfaceArea() const;

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb1, const AxisAlignedBoundingBox& bb2);

    static AxisAlignedBoundingBox Union(const AxisAlignedBoundingBox& bb, const Vector3_t<FLOAT>& p);

    bool IntersectBox(const Ray &ray, const Vector3_t<FLOAT> &invDir,
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
