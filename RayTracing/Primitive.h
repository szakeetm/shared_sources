//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_RTPRIMITIVE_H
#define MOLFLOW_PROJ_RTPRIMITIVE_H

#include "BoundingBox.h"

class Ray;

// General ray tracing interface
class RTPrimitive {
public:
    virtual ~RTPrimitive() = default;
    virtual void ComputeBB() = 0;
    virtual bool Intersect(Ray &r) = 0;

    AxisAlignedBoundingBox bb;
};
#endif //MOLFLOW_PROJ_RTPRIMITIVE_H
