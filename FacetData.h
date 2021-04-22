//
// Created by pbahr on 08/02/2021.
//

#ifndef MOLFLOW_PROJ_FACETDATA_H
#define MOLFLOW_PROJ_FACETDATA_H

#include <vector>
#include <RayTracing/Primitive.h>
#include "Vector.h"
#include "Buffer_shared.h"

class Surface {
public:
    bool IsHardHit(const Ray &r) { return false; };
};

struct Facet : public RTPrimitive {
    Facet() : RTPrimitive(), sh(0){ surf = new Surface(); };
    Facet(size_t nbIndex) : RTPrimitive(), sh(nbIndex) { surf = new Surface(); };
    ~Facet(){
        if (surf) {
            delete surf;
            surf = nullptr;}
    }
    FacetProperties sh;
    std::vector<size_t>      indices;          // Indices (Reference to geometry vertex)
    std::vector<Vector2d> vertices2;        // Vertices (2D plane space, UV coordinates)
    Surface* surf;
    size_t globalId; //Global index (to identify when superstructures are present)

    void ComputeBB() { bb = sh.bb;};
    bool Intersect(Ray &r) const override;

};


#endif //MOLFLOW_PROJ_FACETDATA_H
