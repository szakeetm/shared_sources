//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_RAY_H
#define MOLFLOW_PROJ_RAY_H

#include "Vector.h"

using FLOAT = float;
struct SubProcessFacetTempVar;
class MersenneTwister;

struct HitChain{
    size_t hitId;
    SubProcessFacetTempVar* hit;
    HitChain* next;
};

struct HitLink{
    HitLink(size_t id, SubProcessFacetTempVar* h) : hitId(id), hit(h){};
    size_t hitId;
    SubProcessFacetTempVar* hit;
};

struct Payload {};
constexpr double inf_d = 1.0e99;
class Ray {
public:
    Ray() : tMax(inf_d), time(0.f), structure(-1), lastIntersected(-1), hitChain(nullptr), rng(nullptr), pay(nullptr) {}
    Ray(const Vector3_t<FLOAT> &o, const Vector3_t<FLOAT> &d, Payload* payload, double tMax = inf_d,
        double time = 0.f, int structure = -1)
            : origin(o), direction(d), tMax(tMax), time(time), structure(structure), hitChain(nullptr), rng(nullptr), pay(payload) {}
    ~Ray(){if(pay) delete pay;}
    Vector3_t<FLOAT> operator()(double t) const { return origin + direction * t; }

    Vector3_t<FLOAT> origin;
    Vector3_t<FLOAT> direction;

    // To keep track of shortest intersection
    double tMax;

    double time; // Only for td simulations in Molflow
    int lastIntersected; //
    int structure; //
    //const Medium *medium;
    Payload* pay;

    HitChain* hitChain;
    std::vector<HitLink>* hits;
    MersenneTwister* rng;
};

#endif //MOLFLOW_PROJ_RAY_H
