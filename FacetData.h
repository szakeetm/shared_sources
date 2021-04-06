//
// Created by pbahr on 08/02/2021.
//

#ifndef MOLFLOW_PROJ_FACETDATA_H
#define MOLFLOW_PROJ_FACETDATA_H

#include <vector>
#include "Vector.h"
#include "Buffer_shared.h"

struct Facet {
    Facet() : geo(0), sh() {};
    Facet(size_t nbIndex) : geo(nbIndex), sh() {};
    FacetGeometry geo;
    FacetProperties sh;
    std::vector<size_t>      indices;          // Indices (Reference to geometry vertex)
};


#endif //MOLFLOW_PROJ_FACETDATA_H
