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
    enum class SplitMethod { SAH, HLBVH, Middle, EqualCounts };

    // BVHAccel Public Methods
    BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::SAH);
    bool Intersect(Ray &ray) const;
    ~BVHAccel() override{if(nodes)free(nodes);};

private:
    void ComputeBB() override;
    // BVHAccel Private Methods
    BVHBuildNode *recursiveBuild(
            std::vector<BVHPrimitiveInfo> &primitiveInfo,
            int start, int end, int *totalNodes,
            std::vector<std::shared_ptr<Primitive>> &orderedPrims);
    int flattenBVHTree(BVHBuildNode *node, int *offset);

private:
    const int maxPrimsInNode;
    const SplitMethod splitMethod;
    std::vector<std::shared_ptr<Primitive>> primitives;
    LinearBVHNode *nodes = nullptr;

    int SplitEqualCounts(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim);

    int SplitMiddle(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                    AxisAlignedBoundingBox &centroidBounds);

    int SplitSAH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                 AxisAlignedBoundingBox &centroidBounds, AxisAlignedBoundingBox &bounds);
};


#endif //MOLFLOW_PROJ_BVH_H
