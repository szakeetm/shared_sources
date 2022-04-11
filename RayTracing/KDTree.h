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

#ifndef MOLFLOW_PROJ_KDTREE_H
#define MOLFLOW_PROJ_KDTREE_H

#include <vector>
#include <memory>
#include <FacetData.h>
#include "Primitive.h"

using Primitive = Facet;

// KdTreeAccel Forward Declarations
struct KdAccelNode;
struct BoundEdge;
class KdTreeAccel : public RTPrimitive {
public:
    // KdTreeAccel Public Methods
    KdTreeAccel(std::vector<std::shared_ptr<Primitive>> p, const std::vector<double>& probabilities = std::vector<double>{},
                int isectCost = 80, int traversalCost = 1,
                double emptyBonus = 0.5, int maxPrims = 1, int maxDepth = -1);
    KdTreeAccel(KdTreeAccel && src) noexcept;
    KdTreeAccel(const KdTreeAccel & src) noexcept;

    KdTreeAccel& operator=(const KdTreeAccel & src) noexcept;
    ~KdTreeAccel() override;

    bool Intersect(Ray &ray);

private:
    void ComputeBB() override;
    // KdTreeAccel Private Methods
    void buildTree(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                   const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                   int depth, const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1,
                   int badRefines, const std::vector<double> &probabilities, int prevSplitAxis);

private:
    // KdTreeAccel Private Data
    const int isectCost, traversalCost, maxPrims;
    const double emptyBonus;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<int> primitiveIndices;
    KdAccelNode *nodes;
    int nAllocedNodes, nextFreeNode;
    AxisAlignedBoundingBox bounds;
};

struct KdToDo {
    const KdAccelNode *node;
    double tMin, tMax;
};

#endif //MOLFLOW_PROJ_KDTREE_H
