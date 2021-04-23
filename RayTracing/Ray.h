//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_RAY_H
#define MOLFLOW_PROJ_RAY_H

#include "Vector.h"

struct SubProcessFacetTempVar;
class MersenneTwister;

struct HitChain{
    size_t hitId;
    SubProcessFacetTempVar* hit;
    HitChain* next;
};

constexpr double inf_d = 1.0e99;
class Ray {
public:
    Ray() : tMax(inf_d), time(0.f), structure(-1) {}
    Ray(const Vector3d &o, const Vector3d &d, double tMax = inf_d,
        double time = 0.f, int structure = -1)
            : origin(o), direction(d), tMax(tMax), time(time), structure(structure) {}
    Vector3d operator()(double t) const { return origin + direction * t; }

    Vector3d origin;
    Vector3d direction;

    // To keep track of shortest intersection
    double tMax;

    double time; // Only for td simulations in Molflow
    int lastIntersected; //
    int structure; //
    //const Medium *medium;

    HitChain* hitChain;
    MersenneTwister* rng;
};

#endif //MOLFLOW_PROJ_RAY_H
