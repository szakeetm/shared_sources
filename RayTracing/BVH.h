//
// Created by pascal on 4/21/21.
//

#ifndef MOLFLOW_PROJ_BVH_H
#define MOLFLOW_PROJ_BVH_H

#include <vector>
#include <memory>
#include <FacetData.h>
#include <atomic>
#include "Primitive.h"

using Primitive = Facet;

struct BVHBuildNode;

// BVHAccel Forward Declarations
struct BVHPrimitiveInfo;
struct LinearBVHNode {
    AxisAlignedBoundingBox bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    uint16_t nPrimitives;  // 0 -> interior node
    uint8_t axis;          // interior node: xyz
    uint8_t pad[1];        // ensure 32 byte total size
    friend class Geometry;
};

struct IntersectCount{
    IntersectCount() : nbChecks(0), nbIntersects(0){};
    IntersectCount(const IntersectCount&) = delete;
    IntersectCount(IntersectCount&&) = delete;//change this to 'delete' will give a similar compiler error

    std::atomic<size_t> nbChecks{0};
    std::atomic<size_t> nbIntersects{0};
    size_t nbPrim{0};
    size_t level{0};
};

class BVHAccel : public RTPrimitive {
public:
    // BVHAccel Public Types
    enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts, MolflowSplit, ProbSplit
    };

    // BVHAccel Public Methods
    BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::SAH, const std::vector<double>& probabilities = std::vector<double>{});
    BVHAccel(BVHAccel && src) noexcept;
    BVHAccel(const BVHAccel & src) noexcept;

    BVHAccel& operator=(const BVHAccel & src) noexcept;
    ~BVHAccel() override;

    bool Intersect(Ray &ray);

    std::vector<IntersectCount> ints;
    LinearBVHNode* GetNodes(){return nodes;};
private:
    void ComputeBB() override;
    // BVHAccel Private Methods
    BVHBuildNode *recursiveBuild(
            std::vector<BVHPrimitiveInfo> &primitiveInfo,
            int start, int end, int *totalNodes,
            std::vector<std::shared_ptr<Primitive>> &orderedPrims);
    int flattenBVHTree(BVHBuildNode *node, int *offset);
    int flattenBVHTreeStats(BVHBuildNode *node, int *offset, int level);

private:
    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<std::shared_ptr<Primitive>> primitives;
    LinearBVHNode *nodes = nullptr;

    int SplitEqualCounts(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim);

    int SplitMiddle(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                    AxisAlignedBoundingBox &centroidBounds);

    int SplitProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                    AxisAlignedBoundingBox& centroidBounds, AxisAlignedBoundingBox& bounds);

    int SplitSAH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                 AxisAlignedBoundingBox &centroidBounds, AxisAlignedBoundingBox &bounds);

    int SplitMiddleProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim);

    friend class Geometry;
};


#endif //MOLFLOW_PROJ_BVH_H
