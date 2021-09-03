//
// Created by pascal on 4/22/21.
//

#ifndef MOLFLOW_PROJ_RAY_H
#define MOLFLOW_PROJ_RAY_H

#include "Vector.h"
#include "RTHelper.h"

//struct SubProcessFacetTempVar;
class MersenneTwister;

struct HitChain {
    size_t hitId;
    SubProcessFacetTempVar *hit;
    HitChain *next;
};

struct HitLink {
    HitLink(size_t id, SubProcessFacetTempVar h) : hitId(id), hit(h) {};

    // Move constructor called on resize, prevent from deleting SubProcessFacetTempVar
    HitLink(const HitLink &rhs) = default;

    HitLink(HitLink &&rhs) noexcept:
            hitId(rhs.hitId),
            hit(rhs.hit) {};

    HitLink &operator=(const HitLink &src) {
        hitId = src.hitId;
        hit = src.hit;
        return *this;
    };

    HitLink &operator=(HitLink &&src) {
        hitId = src.hitId;
        hit = src.hit;
        return *this;
    };

    ~HitLink();

    size_t hitId;
    SubProcessFacetTempVar hit;
};

struct Payload {
};
constexpr double inf_d = 1.0e99;

class Ray {
public:
    Ray() : tMax(inf_d), time(0.f), structure(-1), lastIntersected(-1), hitChain(nullptr), rng(nullptr), pay(nullptr) {}

    Ray(const Vector3d &o, const Vector3d &d, Payload *payload, double tMax = inf_d,
        double time = 0.f, int structure = -1)
            : origin(o), direction(d), tMax(tMax), time(time), structure(structure), hitChain(nullptr), rng(nullptr),
              pay(payload) {}

    ~Ray() { if (pay) delete pay; }

    Vector3d operator()(double t) const { return origin + direction * t; }

    Vector3d origin;
    Vector3d direction;

    // To keep track of shortest intersection
    double tMax;

    double time; // Only for td simulations in Molflow
    int lastIntersected; //
    int structure; //

    //Statistics
    size_t traversalSteps{0};

    //const Medium *medium;
    Payload *pay;

    HitChain *hitChain;
    std::vector<HitLink> hits;
    MersenneTwister *rng;
};

class RayStat : public Ray {
public:
    RayStat() : Ray() {}

    RayStat(const Vector3d &o, const Vector3d &d, Payload *payload, double tMax = inf_d,
        double time = 0.f, int structure = -1)
            : Ray(o, d, payload, tMax, time, structure) {}

    virtual ~RayStat() = default;

    //Statistics
    size_t traversalSteps{0};
};

#endif //MOLFLOW_PROJ_RAY_H
