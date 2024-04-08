#ifndef MOLFLOW_PROJ_RAY_H
#define MOLFLOW_PROJ_RAY_H

#include "Vector.h"
#include "RTHelper.h"

class MersenneTwister;

//! Keep track of temporary/transparent hits in a single linked list
//! Deprecated as replaced by HitLink
/*
struct HitChain {
    size_t hitId;
    SimulationFacetTempVar *hit;
    HitChain *next;
};
*/

//! Keep track of temporary/transparent hits; correspomds to an individual hit
struct HitLink {
    HitLink() : hitId(9999999999), hit(SimulationFacetTempVar()) {};
    HitLink(size_t id, SimulationFacetTempVar h) : hitId(id), hit(h) {};

    // Move constructor called on resize, prevent from deleting SimulationFacetTempVar
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

    size_t hitId; //! id of the hit entity
    SimulationFacetTempVar hit; //! Hit statistic
};

//! Additional application specific payload
//! Unusued for Molflow

struct Payload {
};

#if defined(SYNRAD)
struct Synpay : public Payload {
    Synpay(double e){energy=e;}
    double energy;
};
#endif

constexpr double inf_d = 1.0e99;

//! Geometric class describing a ray for ray-object intersections in ray-tracing algorithms
class Ray {
public:
    Ray() : tMax(inf_d), time(0.f), structure(-1), lastIntersected(-1), /*hitChain(nullptr),*/ rng(nullptr), pay(nullptr) {}

    Ray(const Vector3d &o, const Vector3d &d, Payload *payload, double tMax = inf_d,
        double time = 0.f, int structure = -1)
            : origin(o), direction(d), tMax(tMax), time(time), structure(structure), /*hitChain(nullptr),*/ rng(nullptr),
              pay(payload) {}

    ~Ray() { if (pay) delete pay; }

    Vector3d operator()(double t) const { return origin + direction * t; }

    Vector3d origin;
    Vector3d direction;

    double tMax;     // To keep track of shortest intersection

    double time; // Only for td simulations in Molflow
    int lastIntersected=-1; // id of last intersected entity
    int structure; // id of structure in which ray currently interacts
    //const Medium *medium;
    Payload *pay;

    std::vector<HitLink> transparentHits;
    HitLink hardHit;
    MersenneTwister *rng;
};

#endif //MOLFLOW_PROJ_RAY_H
