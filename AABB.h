//
// Created by pbahr on 07/04/2021.
//

#ifndef MOLFLOW_PROJ_AABB_H
#define MOLFLOW_PROJ_AABB_H

#include <tuple>
#include <vector>
#include "Geometry_shared.h"

class AxisAlignedBoundingBox;

class AABBNODE {
public:
    AABBNODE();
    ~AABBNODE();
    void ComputeBB();
    std::tuple<size_t, size_t, size_t> FindBestCuttingPlane();
    AxisAlignedBoundingBox             bb;
    AABBNODE *left;
    AABBNODE *right;
    std::vector<SubprocessFacet*> facets;
};

AABBNODE *BuildAABBTree(const std::vector<SubprocessFacet*>& facets,const size_t depth,size_t& maxDepth);

#endif //MOLFLOW_PROJ_AABB_H
