#pragma once

#include "Vector.h"
#include "RTHelper.h"

class MersenneTwister;

//! Keep track of temporary/transparent hits; corresponds to an individual hit
struct HitDescriptor {
    HitDescriptor() : facetId(9999999999), hitDetails(FacetHitDetails()) {};
    HitDescriptor(size_t facetId, FacetHitDetails h) : facetId(facetId), hitDetails(h) {};

    // Move constructor called on resize, prevent from deleting FacetHitDetails
    HitDescriptor(const HitDescriptor &rhs) = default;

    HitDescriptor(HitDescriptor &&rhs) noexcept:
            facetId(rhs.facetId),
            hitDetails(rhs.hitDetails) {};

    HitDescriptor &operator=(const HitDescriptor &src) {
        facetId = src.facetId;
        hitDetails = src.hitDetails;
        return *this;
    };

    HitDescriptor &operator=(HitDescriptor &&src) {
        facetId = src.facetId;
        hitDetails = src.hitDetails;
        return *this;
    };

    ~HitDescriptor();

    size_t facetId; //! id of the hit entity
    FacetHitDetails hitDetails; //! Hit statistic
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
    Ray() : tMax(inf_d), time(0.f), structure(-1), lastIntersectedId(-1), /*hitChain(nullptr),*/ rng(nullptr), pay(nullptr) {}

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
    int lastIntersectedId=-1; // id of last intersected entity
    int structure; // id of structure in which ray currently interacts
    //const Medium *medium;
    Payload *pay;

    std::vector<HitDescriptor> transparentHits;
    HitDescriptor hardHit;
    MersenneTwister *rng;
};
