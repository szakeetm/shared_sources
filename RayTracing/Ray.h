/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#ifndef MOLFLOW_PROJ_RAY_H
#define MOLFLOW_PROJ_RAY_H

#include "Vector.h"
#include "RTHelper.h"

class MersenneTwister;

//! Keep track of temporary/transparent hits in a single linked list
//! Deprecated as replaced by HitLink
struct HitChain {
    size_t hitId;
    SubProcessFacetTempVar *hit;
    HitChain *next;
};

//! Keep track of temporary/transparent hits; correspomds to an individual hit
struct HitLink {
    HitLink() : hitId(9999999999), hit(SubProcessFacetTempVar()) {};
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

    size_t hitId; //! id of the hit entity
    SubProcessFacetTempVar hit; //! Hit statistic
};

//! Additional application specific payload
//! Unusued for Molflow

struct Payload {
};
constexpr double inf_d = 1.0e99;

//! Geometric class describing a ray for ray-object intersections in ray-tracing algorithms
class Ray {
public:
    Ray() : tMax(inf_d), time(0.f), structure(-1), lastIntersected(-1), hitChain(nullptr), rng(nullptr), pay(nullptr) {}

    Ray(const Vector3d &o, const Vector3d &d, Payload *payload, double tMax = inf_d,
        double time = 0.f, int structure = -1)
            : origin(o), direction(d), tMax(tMax), time(time), structure(structure), hitChain(nullptr), rng(nullptr),
              pay(payload) {}

    ~Ray() {
        if (pay) {
            delete pay;
            pay=nullptr;
        }
    }

    Vector3d operator()(double t) const { return origin + direction * t; }

    Vector3d origin;
    Vector3d direction;

    double tMax;     // To keep track of shortest intersection

    double time; // Only for td simulations in Molflow
    int lastIntersected; // id of last intersected entity

    int structure; //id of structure in which ray currently interacts
    //const Medium *medium;
    Payload *pay;

    HitChain *hitChain;
    std::vector<HitLink> hits;
    HitLink hardHit;
    std::vector<HitLink> transparentHits; // TODO: Remove Extra debug structure
    MersenneTwister *rng;
};

struct RayStatistics {
    size_t traversalSteps{0};
    size_t nbIntersectionTests{0};
    size_t nbBoxIntersectionTests{0};
    size_t nbDownTest{0};
    size_t nbUpTest{0};

    RayStatistics& operator+=(const RayStatistics& src) {
        this->traversalSteps += src.traversalSteps;
        this->nbIntersectionTests += src.nbIntersectionTests;
        this->nbBoxIntersectionTests += src.nbBoxIntersectionTests;
        this->nbDownTest += src.nbDownTest;
        this->nbUpTest += src.nbUpTest;

        return *this;
    }

    inline void Reset(){
        traversalSteps = 0;
        nbIntersectionTests = 0;
        nbBoxIntersectionTests = 0;
        nbDownTest = 0;
        nbUpTest = 0;
    }
};

class RayStat : public Ray {
public:
    RayStat() : Ray() {}

    explicit RayStat(const Ray &src) : Ray(src) {}

    RayStat(const Vector3d &o, const Vector3d &d, Payload *payload = nullptr, double tMax = inf_d,
        double time = 0.f, int structure = 0)
            : Ray(o, d, payload, tMax, time, structure) {}

    virtual ~RayStat() = default;
    RayStat &operator=(const RayStat &src) noexcept {
        origin = src.origin;
        direction = src.direction;
        lastIntersected = src.lastIntersected;
        structure = src.structure;
        tMax = src.tMax;
        time = src.time;

        return *this;
    }

    void ResetStats(){
        stats.Reset();
    }

    //Statistics
    RayStatistics stats;
    std::vector<size_t> traversedNodes;
};

#endif //MOLFLOW_PROJ_RAY_H
