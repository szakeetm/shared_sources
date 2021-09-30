//
// Created by pbahr on 08/02/2021.
//

#ifndef MOLFLOW_PROJ_FACETDATA_H
#define MOLFLOW_PROJ_FACETDATA_H

#include <vector>
#include <RayTracing/Primitive.h>
#include "Vector.h"
#include "Buffer_shared.h"
#include <Random.h>
#include <atomic>

class Surface {
public:
    virtual ~Surface() = default;
    virtual bool IsHardHit(const Ray &r);
};

class TransparentSurface : public Surface{
public:
    bool IsHardHit(const Ray &r);
};

class AlphaSurface : public Surface{
    double opacity{1.0};
public:
    AlphaSurface(double opacity) : opacity(opacity){};
    bool IsHardHit(const Ray &r) override;
};

struct Facet : public RTPrimitive {
    Facet() : RTPrimitive(), sh(0){ surf = nullptr; };
    Facet(size_t nbIndex) : RTPrimitive(), sh(nbIndex) { surf = nullptr; };
    ~Facet(){
        if (surf) {
            //delete surf;
            // don' t delete, origin is an unreferenced shared ptr
            surf = nullptr;
        }
    }
    FacetProperties sh;
    std::vector<size_t>      indices;          // Indices (Reference to geometry vertex)
    std::vector<Vector2d> vertices2;        // Vertices (2D plane space, UV coordinates)
    Surface* surf;

    size_t globalId; //Global index (to identify when superstructures are present)
    //size_t iSCount{0};

    // Statistics
    std::atomic<unsigned long long int> nbTraversalSteps{0};
    std::atomic<unsigned long long int> nbIntersections{0};
    std::atomic<unsigned long long int> nbTests{0};

    void ComputeBB() { bb = sh.bb;};
    bool Intersect(Ray &r) override;
    bool IntersectStat(RayStat &r);

};


#endif //MOLFLOW_PROJ_FACETDATA_H
