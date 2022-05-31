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

#ifndef MOLFLOW_PROJ_BVH_H
#define MOLFLOW_PROJ_BVH_H

#include <vector>
#include <memory>
#include <FacetData.h>
#include <atomic>
#include "Primitive.h"

using Primitive = Facet;

namespace Profiling {
    extern CummulativeBenchmark intersectStatsBVH;
}

// BVHAccel Forward Declarations
struct BVHBuildNode;
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

struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() : primitiveNumber(0), bounds(),
                         centroid(), probability(0.0) {}

    BVHPrimitiveInfo(size_t primitiveNumber, const AxisAlignedBoundingBox &bounds)
            : primitiveNumber(primitiveNumber), bounds(bounds),
              centroid(.5f * bounds.min + .5f * bounds.max),
              probability(0.0) {}

    BVHPrimitiveInfo(size_t primitiveNumber, const AxisAlignedBoundingBox &bounds, double probability)
            : primitiveNumber(primitiveNumber), bounds(bounds),
              centroid(.5f * bounds.min + .5f * bounds.max),
              probability(probability) {}

    size_t primitiveNumber;
    AxisAlignedBoundingBox bounds;
    Vector3d centroid;
    double probability; // For MCHitSplit
};

class BVHAccel : public RTAccel {
public:
    // BVHAccel Public Types
    enum class SplitMethod {
        SAH, HLBVH, Middle, EqualCounts, MolflowSplit, ProbSplit, RDH
    };
    friend std::ostream& operator << (std::ostream& os, SplitMethod split_type);

    // BVHAccel Public Methods
    BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::SAH);
    BVHAccel(const std::vector<double> &probabilities,
             std::vector<std::shared_ptr<Primitive>> p, int maxPrimsInNode = 1,
             SplitMethod splitMethod = SplitMethod::ProbSplit);
    BVHAccel(const std::vector<TestRay> &battery, std::vector<std::shared_ptr<Primitive>> p,
             int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::RDH);
    BVHAccel(BVHAccel && src) noexcept;
    BVHAccel(const BVHAccel & src) noexcept;

    BVHAccel& operator=(const BVHAccel & src) noexcept;
    ~BVHAccel() override;

    bool Intersect(Ray &ray) override;

    LinearBVHNode* GetNodes(){return nodes;};
private:
    void ComputeBB() override;
    // BVHAccel Private Methods
    void construct(std::vector<BVHPrimitiveInfo> primitiveInfo);
    BVHBuildNode *recursiveBuild(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int *totalNodes,
                                 std::vector<std::shared_ptr<Primitive>> &orderedPrims,
                                 std::vector<TestRay> &testBattery,
                                 const std::vector<TestRayLoc> &local_battery);
    int flattenBVHTree(BVHBuildNode *node, int *offset);
    int flattenBVHTreeStats(BVHBuildNode *node, int *offset, int level);

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

    int SplitRDH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim,
                 AxisAlignedBoundingBox &centroidBounds, std::vector<TestRay> &local_battery,
                 AxisAlignedBoundingBox &bounds);

    int SplitMiddleProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start, int end, int dim);

    friend class Geometry;

    bool IntersectStat(RayStat &ray) override;

};

#endif //MOLFLOW_PROJ_BVH_H
