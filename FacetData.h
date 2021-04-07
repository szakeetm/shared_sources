//
// Created by pbahr on 08/02/2021.
//

#ifndef MOLFLOW_PROJ_FACETDATA_H
#define MOLFLOW_PROJ_FACETDATA_H

#include <vector>
#include "Vector.h"
#include "Buffer_shared.h"

struct Facet {
    Facet() : sh(0) {};
    Facet(size_t nbIndex) : sh(nbIndex) {};
    FacetProperties sh;
    std::vector<size_t>      indices;          // Indices (Reference to geometry vertex)
    std::vector<Vector2d> vertices2;        // Vertices (2D plane space, UV coordinates)
};


#endif //MOLFLOW_PROJ_FACETDATA_H
