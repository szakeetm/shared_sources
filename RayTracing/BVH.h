//
// Created by pascal on 4/21/21.
//

#ifndef MOLFLOW_PROJ_BVH_H
#define MOLFLOW_PROJ_BVH_H

#include <vector>
#include <memory>
#include <FacetData.h>
#include "Primitive.h"

using Primitive = Facet;

struct BVHBuildNode;

// BVHAccel Forward Declarations
struct BVHPrimitiveInfo;
struct LinearBVHNode;

class BVHAccel : public RTPrimitive {
public:
    // BVHAccel Public Types
    enum class SplitMethod {
        SAH, HLBVH, Middle, EqualCounts, MolflowSplit, ProbSplit, TestSplit
    };

    // BVHAccel Public Methods
    BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::SAH);
    BVHAccel(const std::vector<double> &probabilities,
             std::vector<std::shared_ptr<Primitive>> p, int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::ProbSplit);
    BVHAccel(const std::vector<TestRay> &battery, std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::TestSplit);
    BVHAccel(BVHAccel && src) noexcept;
    BVHAccel(const BVHAccel & src) noexcept;

    BVHAccel& operator=(const BVHAccel & src) noexcept;
    ~BVHAccel() override;

    bool Intersect(Ray &ray) override;

private:
    void ComputeBB() override;
    // BVHAccel Private Methods
    void construct(std::vector<BVHPrimitiveInfo> primitiveInfo = std::vector<BVHPrimitiveInfo>());
    BVHBuildNode *recursiveBuild(
            std::vector<BVHPrimitiveInfo> &primitiveInfo,
            int start, int end, int *totalNodes,
            std::vector<std::shared_ptr<Primitive>> &orderedPrims);
    int flattenBVHTree(BVHBuildNode *node, int *offset);

private:
    const int maxPrimsInNode;
    SplitMethod splitMethod;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<TestRay> battery;

    LinearBVHNode *nodes = nullptr;

    int SplitEqualCounts(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim);

    int SplitMiddle(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                    AxisAlignedBoundingBox &centroidBounds);

    int SplitProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                    AxisAlignedBoundingBox& centroidBounds, AxisAlignedBoundingBox& bounds);

    int SplitSAH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                 AxisAlignedBoundingBox &centroidBounds, AxisAlignedBoundingBox &bounds);

    int SplitMiddleProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim);
};


#endif //MOLFLOW_PROJ_BVH_H
