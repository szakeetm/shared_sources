//
// Created by pascal on 4/21/21.
//

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
    void buildTree(int nodeNum, const AxisAlignedBoundingBox &bounds,
                   const std::vector<AxisAlignedBoundingBox> &primBounds, int *primNums,
                   int nprims, int depth,
                   const std::unique_ptr<BoundEdge[]> edges[3], int *prims0,
                   int *prims1, int badRefines = 0, const std::vector<double>& probabilities = std::vector<double>{});

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
