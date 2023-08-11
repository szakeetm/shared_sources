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

/*
 * BVH code based on pbrt-v3
 * Copyright (c) 1998-2015, Matt Pharr, Greg Humphreys, and Wenzel Jakob.
 */


#include <BoundingBox.h>
#include "BVH.h"
#include "Ray.h"
#include <cassert>
#include <Helper/ConsoleLogger.h>
#include "IntersectAABB_shared.h"

namespace STATS {
    //STAT_MEMORY_COUNTER("Memory/BVH tree", treeBytes);
    //STAT_RATIO("BVH/Primitives per leaf node", totalPrimitives, totalLeafNodes);

    static int totalPrimitives = 0;
    static int totalLeafNodes = 0;
    static int interiorNodes = 0;
    static int leafNodes = 0;

    void _reset() {
        totalPrimitives = 0;
        totalLeafNodes = 0;
        interiorNodes = 0;
        leafNodes = 0;
    }
}
struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() : primitiveNumber(0), bounds(),
                         centroid(), probability(0.0) {}

    BVHPrimitiveInfo(int primitiveNumber, const AxisAlignedBoundingBox &bounds)
            : primitiveNumber(primitiveNumber), bounds(bounds),
              centroid(.5f * bounds.min + .5f * bounds.max),
              probability(0.0) {}

    BVHPrimitiveInfo(int primitiveNumber, const AxisAlignedBoundingBox &bounds, double probability)
            : primitiveNumber(primitiveNumber), bounds(bounds),
              centroid(.5f * bounds.min + .5f * bounds.max),
              probability(probability) {}

    int primitiveNumber;
    AxisAlignedBoundingBox bounds;
    Vector3d centroid;
    double probability; // For MCHitSplit
};

struct BVHBuildNode {
    // BVHBuildNode Public Methods
    void InitLeaf(int first, int n, const AxisAlignedBoundingBox &b) {
        firstPrimOffset = first;
        nPrimitives = n;
        bounds = b;
        children[0] = children[1] = nullptr;
        ++STATS::leafNodes;
        ++STATS::totalLeafNodes;
        STATS::totalPrimitives += n;
    }

    void InitInterior(int axis, BVHBuildNode *c0, BVHBuildNode *c1) {
        children[0] = c0;
        children[1] = c1;
        bounds = AxisAlignedBoundingBox::Union(c0->bounds, c1->bounds);
        splitAxis = axis;
        nPrimitives = 0;
        ++STATS::interiorNodes;
    }

    ~BVHBuildNode(){
        if(children[1]) delete children[1];
        if(children[0]) delete children[0];
    }
    AxisAlignedBoundingBox bounds;
    BVHBuildNode *children[2];
    int splitAxis, firstPrimOffset, nPrimitives;
};

struct LinearBVHNode {
    AxisAlignedBoundingBox bounds;
    union {
        int primitivesOffset;   // leaf
        int secondChildOffset;  // interior
    };
    uint16_t nPrimitives;  // 0 -> interior node
    uint8_t axis;          // interior node: xyz
    uint8_t pad[1];        // ensure 32 byte total size
};

int BVHAccel::SplitEqualCounts(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
                               int end, int dim) {
    //<<Partition primitives into equally sized subsets>>
    int mid = (start + end) / 2;
    std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
                     &primitiveInfo[end - 1] + 1,
                     [dim](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) {
                         return a.centroid[dim] < b.centroid[dim];
                     });

    return mid;
}

int BVHAccel::SplitMiddle(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
                          int end, int dim, AxisAlignedBoundingBox &centroidBounds) {
    //<<Partition primitives through node’s midpoint>>
    double pmid = (centroidBounds.min[dim] + centroidBounds.max[dim]) / 2;
    BVHPrimitiveInfo *midPtr =
            std::partition(&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
                           [dim, pmid](const BVHPrimitiveInfo &pi) {
                               return pi.centroid[dim] < pmid;
                           });
    int mid = midPtr - &primitiveInfo[0];
    if (mid != start && mid != end)
        return mid;

    return SplitEqualCounts(primitiveInfo, start, end, dim);
}

int BVHAccel::SplitMiddleProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
                              int end, int dim) {
    //<<Partition primitives through node’s midpoint>>
    std::sort(&primitiveInfo[start], &primitiveInfo[end],
              [](const BVHPrimitiveInfo &a, const BVHPrimitiveInfo &b) {
                  return a.probability < b.probability;
              });

    double sumProb = 0.0;
    for (int i = start; i < end; ++i) {
        sumProb += primitiveInfo[i].probability;
    }
    sumProb *= 0.5; // take half as split point

    double midProb = 0.0;
    int mid = start;
    for (; mid < end; mid += 2) {
        midProb += primitiveInfo[mid].probability;
        if (midProb > sumProb) break;
    }

    // If mid is valid, make swaps and return point
    if (mid != start && mid < end) {
        for (int i = start + 1, j = mid; i < mid && j < end; i += 2, j += 2) {
            std::swap(primitiveInfo[i], primitiveInfo[j]);
        }
        return mid;
    }

    return SplitEqualCounts(primitiveInfo, start, end, dim);
}

int BVHAccel::SplitProb(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
                        int end, int dim, AxisAlignedBoundingBox &centroidBounds, AxisAlignedBoundingBox &bounds) {
    //<<Partition primitives using approximate SAH>>
    int nPrimitives = end - start;
    int mid = -1;
    if (nPrimitives <= 2) {
        //<<Partition primitives into equally sized subsets>>
        mid = SplitEqualCounts(primitiveInfo, start, end, dim);
    } else if (nPrimitives > maxPrimsInNode) { // else return -1 to just create a leaf node
        //<<Allocate BucketInfo for PROB partition buckets>>
        constexpr int nBuckets = 12;
        struct BucketInfo {
            int count = 0;
            double sumProb = 0.0;
            AxisAlignedBoundingBox bounds;
        };
        BucketInfo buckets[nBuckets];
        double maxProb = 0.0;
        double minProb = 1.0;
        //<<Initialize BucketInfo for PROB partition buckets>>
        for (int i = start; i < end; ++i) {
            int b = nBuckets *
                    centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
            if (b == nBuckets) b = nBuckets - 1;
            assert(b >= 0);
            assert(b < nBuckets);
            buckets[b].count++;
            buckets[b].sumProb += primitiveInfo[i].probability;
            maxProb = std::max(maxProb, primitiveInfo[i].probability);
            minProb = std::min(minProb, primitiveInfo[i].probability);
            buckets[b].bounds = AxisAlignedBoundingBox::Union(buckets[b].bounds, primitiveInfo[i].bounds);
        }

        //<<Compute costs for splitting after each bucket>>
        double cost[nBuckets - 1];
        double cost_min[nBuckets - 1];
        double cost_mid[nBuckets - 1];
        double cost_sah[nBuckets - 1];

        double sumProb = 0.0;
        for (int i = 0; i < nBuckets - 1; ++i) {
            sumProb += buckets[i].sumProb;
        }

        int bestProbBucket = 0;
        int bestCount = 0;
        double prob = 0.0;
        for (int i = 0; i < nBuckets - 1; ++i) {
            prob += buckets[i].sumProb;
            bestCount += buckets[i].count;
            if (prob > sumProb * 0.5) {
                bestProbBucket = i;
                break;
            }
        }

        for (int i = 0; i < nBuckets - 1; ++i) {
            double prob0 = 0.0, prob1 = 0.0;
            AxisAlignedBoundingBox b0, b1;
            int count0 = 0, count1 = 0;
            for (int j = 0; j <= i; ++j) {
                count0 += buckets[j].count;
                prob0 += buckets[j].sumProb;
                b0 = AxisAlignedBoundingBox::Union(b0, buckets[j].bounds);
            }
            for (int j = i + 1; j < nBuckets; ++j) {
                count1 += buckets[j].count;
                prob1 += buckets[j].sumProb;
                b1 = AxisAlignedBoundingBox::Union(b1, buckets[j].bounds);
            }
            cost[i] = 0.5f + std::abs(prob0 - prob1) / maxProb;
            cost_min[i] = 0.5f + std::abs(prob0 - prob1) / minProb;
            cost_mid[i] = 0.5f + std::abs(prob0 - prob1) / (maxProb - minProb);
            cost_sah[i] = 0.5f + (count0 * b0.SurfaceArea() +
                                  count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
        }

        //<<Find bucket to split at that minimizes PROB metric>>
        double minCost = cost_sah[0];
        int minCostSplitBucket = 0;
        for (int i = 1; i < nBuckets - 1; ++i) {
            if (cost_sah[i] < minCost) {
                minCost = cost_sah[i];
                minCostSplitBucket = i;
            }
        }
        //<<Either create leaf or split primitives at selected PROB bucket>>
        double leafCost = nPrimitives;
        if(sumProb <= 0.0) {
            if (minCost < leafCost) {
                BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],
                                                        &primitiveInfo[end - 1] + 1,
                                                        [=](const BVHPrimitiveInfo &pi) {
                                                            int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
                                                            if (b == nBuckets) b = nBuckets - 1;
                                                            assert(b >= 0);
                                                            assert(b < nBuckets);
                                                            return b <= minCostSplitBucket;
                                                        });
                mid = pmid - &primitiveInfo[0];
            }
        }
        else{
            if (cost_sah[bestProbBucket] < leafCost) {
                BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],
                                                        &primitiveInfo[end - 1] + 1,
                                                        [=](const BVHPrimitiveInfo &pi) {
                                                            int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
                                                            if (b == nBuckets) b = nBuckets - 1;
                                                            assert(b >= 0);
                                                            assert(b < nBuckets);
                                                            return b <= bestProbBucket;
                                                        });
                mid = pmid - &primitiveInfo[0];
            }
        }
    }
    return mid;
}

int BVHAccel::SplitSAH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
                       int end, int dim, AxisAlignedBoundingBox &centroidBounds, AxisAlignedBoundingBox &bounds) {
    //<<Partition primitives using approximate SAH>>
    int nPrimitives = end - start;
    int mid = -1;
    if (nPrimitives <= 2) {
        //<<Partition primitives into equally sized subsets>>
        mid = SplitEqualCounts(primitiveInfo, start, end, dim);
    } else if (nPrimitives > maxPrimsInNode) { // else return -1 to just create a leaf node
        //<<Allocate BucketInfo for SAH partition buckets>>
        constexpr int nBuckets = 12;
        struct BucketInfo {
            int count = 0;
            AxisAlignedBoundingBox bounds;
        };
        BucketInfo buckets[nBuckets];
        //<<Initialize BucketInfo for SAH partition buckets>>
        for (int i = start; i < end; ++i) {
            int b = nBuckets *
                    centroidBounds.Offset(primitiveInfo[i].centroid)[dim];
            if (b == nBuckets) b = nBuckets - 1;
            assert(b >= 0);
            assert(b < nBuckets);
            buckets[b].count++;
            buckets[b].bounds = AxisAlignedBoundingBox::Union(buckets[b].bounds, primitiveInfo[i].bounds);
        }

        //<<Compute costs for splitting after each bucket>>
        double cost[nBuckets - 1];
        for (int i = 0; i < nBuckets - 1; ++i) {
            AxisAlignedBoundingBox b0, b1;
            int count0 = 0, count1 = 0;
            for (int j = 0; j <= i; ++j) {
                b0 = AxisAlignedBoundingBox::Union(b0, buckets[j].bounds);
                count0 += buckets[j].count;
            }
            for (int j = i + 1; j < nBuckets; ++j) {
                b1 = AxisAlignedBoundingBox::Union(b1, buckets[j].bounds);
                count1 += buckets[j].count;
            }
            cost[i] = 0.5f + (count0 * b0.SurfaceArea() +
                              count1 * b1.SurfaceArea()) / bounds.SurfaceArea();
        }
        //<<Find bucket to split at that minimizes SAH metric>>
        double minCost = cost[0];
        int minCostSplitBucket = 0;
        for (int i = 1; i < nBuckets - 1; ++i) {
            if (cost[i] < minCost) {
                minCost = cost[i];
                minCostSplitBucket = i;
            }
        }
        //<<Either create leaf or split primitives at selected SAH bucket>>
        double leafCost = nPrimitives;
        if (minCost < leafCost) {
            BVHPrimitiveInfo *pmid = std::partition(&primitiveInfo[start],
                                                    &primitiveInfo[end - 1] + 1,
                                                    [=](const BVHPrimitiveInfo &pi) {
                                                        int b = nBuckets * centroidBounds.Offset(pi.centroid)[dim];
                                                        if (b == nBuckets) b = nBuckets - 1;
                                                        assert(b >= 0);
                                                        assert(b < nBuckets);
                                                        return b <= minCostSplitBucket;
                                                    });
            mid = pmid - &primitiveInfo[0];
        }
    }
    return mid;
}

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> p,
                   int maxPrimsInNode, SplitMethod splitMethod,
                   const std::vector<double> &probabilities)
        : maxPrimsInNode(std::min(255, maxPrimsInNode)),
          splitMethod(splitMethod),
          primitives(std::move(p)) {

    nodes = nullptr;
    if (primitives.empty())
        return;
    STATS::_reset();

    //<<Build BVH from primitives>>
    //<<Initialize primitiveInfo array for primitives>>

    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    if (splitMethod == SplitMethod::ProbSplit) {
        if (primitives.size() > probabilities.size())
            return;
        for (int i = 0; i < primitives.size(); ++i)
            primitiveInfo[i] = {i, primitives[i]->sh.bb, probabilities[primitives[i]->globalId]};
    } else {
        for (int i = 0; i < primitives.size(); ++i)
            primitiveInfo[i] = {i, primitives[i]->sh.bb};
    }
    //<<Build BVH tree for primitives using primitiveInfo>>
    //MemoryArena arena(1024 * 1024);
    int totalNodes = 0;
    std::vector<std::shared_ptr<Primitive>> orderedPrims;
    BVHBuildNode *root;
    /*if (splitMethod == SplitMethod::HLBVH)
        root = HLBVHBuild(arena, primitiveInfo, &totalNodes, orderedPrims);
    else
        */
    root = recursiveBuild(/*arena,*/ primitiveInfo, 0, primitives.size(),
                                     &totalNodes, orderedPrims);
    primitives.swap(orderedPrims);

    Log::console_msg_master(4, "BVH created with {} nodes for {} "
           "primitives ({:.2f} MB)\n",
           totalNodes, (int) primitives.size(),
           float(totalNodes * sizeof(LinearBVHNode)) /
           (1024.f * 1024.f));
    //<<Compute representation of depth-first traversal of BVH tree>>
    nodes = new LinearBVHNode[totalNodes]; //AllocAligned<LinearBVHNode>(totalNodes);
    int offset = 0;
    flattenBVHTree(root, &offset);
    assert(totalNodes == offset);

    delete root;

    Log::console_msg_master(4,"--- BVH STATS ---\n");
    Log::console_msg_master(4," Total Primitives: {}\n", STATS::totalPrimitives);
    Log::console_msg_master(4," Total Leaf Nodes: {}\n", STATS::totalLeafNodes);
    Log::console_msg_master(4," Interior Nodes:   {}\n", STATS::interiorNodes);
    Log::console_msg_master(4," Leaf Nodes:       {}\n", STATS::leafNodes);

}

BVHAccel::~BVHAccel() {
    if (nodes) {
        delete[] nodes;
        nodes = nullptr;
    }
};

BVHBuildNode *BVHAccel::recursiveBuild(
        std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
        int end, int *totalNodes,
        std::vector<std::shared_ptr<Primitive>> &orderedPrims) {
    assert(start != end);

    BVHBuildNode *node = new BVHBuildNode();//arena.Alloc<BVHBuildNode>();
    (*totalNodes)++;
    //<<Compute bounds of all primitives in BVH node>>
    AxisAlignedBoundingBox bounds;
    for (int i = start; i < end; ++i)
        bounds = AxisAlignedBoundingBox::Union(bounds, primitiveInfo[i].bounds);

    int nPrimitives = end - start;
    if (nPrimitives == 1) {
        //<<Create leaf BVHBuildNode>>
        int firstPrimOffset = orderedPrims.size();
        for (int i = start; i < end; ++i) {
            int primNum = primitiveInfo[i].primitiveNumber;
            orderedPrims.push_back(primitives[primNum]);
        }
        node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
        return node;
    } else {
        //<<Compute bound of primitive centroids, choose split dimension dim>>
        AxisAlignedBoundingBox centroidBounds;
        for (int i = start; i < end; ++i)
            centroidBounds = AxisAlignedBoundingBox::Union(centroidBounds, primitiveInfo[i].centroid);
        int dim = centroidBounds.MaximumExtent();

        //<<Partition primitives into two sets and build children>>
        int mid = (start + end) / 2;
        if (centroidBounds.max[dim] == centroidBounds.min[dim]) {
            //<<Create leaf BVHBuildNode>>
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i) {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(primitives[primNum]);
            }
            node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
            return node;
        } else {
            //<<Partition primitives based on splitMethod>>
            switch (splitMethod) {
                case SplitMethod::MolflowSplit: {
                    // Mix of Middle and EqualCounts
                    // Find best middle cut (most equal counts) in all dimensions
                    int tmpMid = 0;
                    double bestDeviation = 1.0e99;
                    for (auto itDim : {0, 1, 2}) {
                        tmpMid = SplitMiddle(primitiveInfo, start, end, itDim, centroidBounds);
                        double dev = std::abs((double) tmpMid - (double) ((end - start) / 2.0 + start));
                        if (dev < bestDeviation) {
                            bestDeviation = dev;
                            mid = tmpMid;
                            dim = itDim;
                        }
                    }
                    break;
                }
                case SplitMethod::Middle: {
                    mid = SplitMiddle(primitiveInfo, start, end, dim, centroidBounds);
                    break;
                }
                case SplitMethod::EqualCounts: {
                    //<<Partition primitives into equally sized subsets>>
                    mid = SplitEqualCounts(primitiveInfo, start, end, dim);
                    break;
                }
                case SplitMethod::ProbSplit: {
                    //<<Partition primitives into equally sized subsets>>
                    mid = SplitProb(primitiveInfo, start, end, dim, centroidBounds, bounds);
                    //mid = SplitMiddleProb(primitiveInfo, start, end, dim);
                    break;
                }
                case SplitMethod::SAH:
                default: {
                    mid = SplitSAH(primitiveInfo, start, end, dim, centroidBounds, bounds);
                    break;
                }
            }

            if (mid < 0 || nPrimitives <= maxPrimsInNode) {
                // Create leaf node, when max prims are reached
                int firstPrimOffset = orderedPrims.size();
                for (int i = start; i < end; ++i) {
                    int primNum = primitiveInfo[i].primitiveNumber;
                    orderedPrims.push_back(primitives[primNum]);
                }
                node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
                return node;
            }

            node->InitInterior(dim,
                               recursiveBuild(primitiveInfo, start, mid,
                                              totalNodes, orderedPrims),
                               recursiveBuild(primitiveInfo, mid, end,
                                              totalNodes, orderedPrims));
        }
    }
    return node;
}

int BVHAccel::flattenBVHTree(BVHBuildNode *node, int *offset) {
    LinearBVHNode *linearNode = &nodes[*offset];
    linearNode->bounds = node->bounds;
    int myOffset = (*offset)++;
    if (node->nPrimitives > 0) {
        assert(!node->children[0] && !node->children[1]);
        assert(node->nPrimitives < 65536);
        linearNode->primitivesOffset = node->firstPrimOffset;
        linearNode->nPrimitives = node->nPrimitives;
    } else {
        // Create interior flattened BVH node
        linearNode->axis = node->splitAxis;
        linearNode->nPrimitives = 0;
        flattenBVHTree(node->children[0], offset);
        linearNode->secondChildOffset =
                flattenBVHTree(node->children[1], offset);
    }
    return myOffset;
}

void BVHAccel::ComputeBB() {
    bb = nodes ? nodes[0].bounds : AxisAlignedBoundingBox();
}

bool BVHAccel::Intersect(Ray &ray) {
    if (!nodes) return false;

    bool hit = false;
    Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
    // Follow ray through BVH nodes to find primitive intersections
    int toVisitOffset = 0, currentNodeIndex = 0;
    int nodesToVisit[64];
    while (true) {
        const LinearBVHNode *node = &nodes[currentNodeIndex];
        // Check ray against BVH node
        if (IntersectBox(node->bounds,ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
                // Intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->nPrimitives; ++i) {

                    const std::shared_ptr<Primitive> &p = primitives[node->primitivesOffset + i];
                    // Do not check last collided facet to prevent self intersections
                    if (p->globalId != ray.lastIntersected && p->Intersect(ray)) {
                        hit = true;
                    }
                }
                if (toVisitOffset == 0) break;
                currentNodeIndex = nodesToVisit[--toVisitOffset];
            } else {
                // Put far BVH node on _nodesToVisit_ stack, advance to near
                // node
                if (dirIsNeg[node->axis]) {
                    nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
                    currentNodeIndex = node->secondChildOffset;
                } else {
                    nodesToVisit[toVisitOffset++] = node->secondChildOffset;
                    currentNodeIndex = currentNodeIndex + 1;
                }
            }
        } else {
            if (toVisitOffset == 0) break;
            currentNodeIndex = nodesToVisit[--toVisitOffset];
        }
    }
    return hit;
}

BVHAccel::BVHAccel(BVHAccel &&src) noexcept: maxPrimsInNode(src.maxPrimsInNode), splitMethod(src.splitMethod) {
    primitives = std::move(src.primitives);
    nodes = src.nodes;
    src.nodes = nullptr;
    bb = src.bb;
}

BVHAccel::BVHAccel(const BVHAccel &src) noexcept: maxPrimsInNode(src.maxPrimsInNode), splitMethod(src.splitMethod) {
    if (nodes)
        exit(44);
}

BVHAccel &BVHAccel::operator=(const BVHAccel &src) noexcept {
    if (nodes)
        exit(44);

    return *this;
}

