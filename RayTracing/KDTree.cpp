//
// Created by pascal on 4/21/21.
//

/*
 * KDTree code based on pbrt-v3
 * Copyright (c) 1998-2015, Matt Pharr, Greg Humphreys, and Wenzel Jakob.
 */


#include <BoundingBox.h>
#include <cmath>
#include <memory>
#include <set>
#include <cassert>
#include <random>
#include <Helper/Chronometer.h>

#include "KDTree.h"
#include "Ray.h"

namespace STATS {
    //STAT_MEMORY_COUNTER("Memory/BVH tree", treeBytes);
    //STAT_RATIO("BVH/Primitives per leaf node", totalPrimitives, totalLeafNodes);

    static int totalPrimitives = 0;
    static int totalLeafNodes = 0;
    static int interiorNodes = 0;
    static int leafNodes = 0;
}

inline int Log2Int(uint32_t v) {
#if defined(_MSC_VER)
    unsigned long lz = 0;
    if (_BitScanReverse(&lz, v)) return lz;
    return 0;
#else
    return 31 - __builtin_clz(v);
#endif
}

inline int Log2Int(int32_t v) { return Log2Int((uint32_t) v); }

inline int Log2Int(uint64_t v) {
#if defined(_MSC_VER)
    unsigned long lz = 0;
#if defined(_WIN64)
    _BitScanReverse64(&lz, v);
#else
    if  (_BitScanReverse(&lz, v >> 32))
        lz += 32;
    else
        _BitScanReverse(&lz, v & 0xffffffff);
#endif // _WIN64
    return lz;
#else  // _MSC_VER
    return 63 - __builtin_clzll(v);
#endif
}

inline int Log2Int(int64_t v) { return Log2Int((uint64_t) v); }

namespace STATS_KD {
    //STAT_MEMORY_COUNTER("Memory/BVH tree", treeBytes);
    //STAT_RATIO("BVH/Primitives per leaf node", totalPrimitives, totalLeafNodes);

    static int totalPrimitives = 0;
    static int totalLeafNodes = 0;
    static int interiorNodes = 0;
    static int leafNodes = 0;
    static int leafBadRefine = 0;
    static int leafHigherCost = 0;
    static int splitRay = 0;
    static int splitSAH = 0;

    void _reset() {
        totalPrimitives = 0;
        totalLeafNodes = 0;
        interiorNodes = 0;
        leafNodes = 0;
        leafBadRefine = 0;
        leafHigherCost = 0;
        splitRay = 0;
        splitSAH = 0;
    }
}

void KdAccelNode::InitInterior(int axis, int ac, double s) {
    split = s;
    flags = axis;
    aboveChild |= (ac << 2);
    STATS_KD::interiorNodes++;
}

enum class EdgeType {
    Start, End
};

struct BoundEdge {
    // BoundEdge Public Methods
    BoundEdge() = default;

    BoundEdge(double t, int primNum, bool starting) : t(t), primNum(primNum) {
        type = starting ? EdgeType::Start : EdgeType::End;
    }

    double t{};
    int primNum{};
    EdgeType type;
};

void KdTreeAccel::PrintTreeInfo(){
    std::string splitName;
    switch (splitMethod) {
        case(SplitMethod::SAH):
            splitName = "SAH";
            break;
        case(SplitMethod::ProbSplit):
            splitName = "Prob";
            break;
        case(SplitMethod::TestSplit):
            splitName = "Test";
            break;
        case(SplitMethod::HybridSplit):
            splitName = "Hybrid";
            break;
        default:
            splitName = "Unknown split";
    }
    printf("--- KD STATS ---\n");
    printf("- Split: %s -\n", splitName.c_str());
    printf(" Total Primitives: %d\n", STATS_KD::totalPrimitives);
    printf(" Total Leaf Nodes: %d\n", STATS_KD::totalLeafNodes);
    printf(" Interior Nodes:   %d\n", STATS_KD::interiorNodes);
    printf(" Leaf Nodes:       %d\n", STATS_KD::leafNodes);
    printf(" Leaf 4 BadRefine: %d [%lf]\n", STATS_KD::leafBadRefine, (double)STATS_KD::leafBadRefine/STATS_KD::leafNodes);
    printf(" Leaf 4 BadCost:   %d [%lf]\n", STATS_KD::leafHigherCost, (double)STATS_KD::leafHigherCost/STATS_KD::leafNodes);
    printf(" Split SAH:        %d [%lf]\n", STATS_KD::splitSAH, (double)STATS_KD::splitSAH/(STATS_KD::splitRay+STATS_KD::splitSAH));
    printf(" Split Ray:        %d [%lf]\n", STATS_KD::splitRay, (double)STATS_KD::splitRay/(STATS_KD::splitRay+STATS_KD::splitSAH));
}

void KdTreeAccel::ComputeBB() {
    bb = AxisAlignedBoundingBox();
    if (nodes) {
        for (const auto& prim : primitives) {
            bb = AxisAlignedBoundingBox::Union(bb, prim->bb);
        }
    }
}

// KdTreeAccel Method Definitions
KdTreeAccel::KdTreeAccel(SplitMethod splitMethod, std::vector<std::shared_ptr<Primitive>> p,
                         const std::vector<double> &probabilities, const std::vector<TestRay> &battery,
                         int isectCost, int traversalCost, double emptyBonus, int maxPrims, int maxDepth)
        : splitMethod(splitMethod),
          isectCost(isectCost),
          traversalCost(traversalCost),
          maxPrims(maxPrims),
          emptyBonus(emptyBonus),
          primitives(std::move(p)) {

    nodes = nullptr;
    if (primitives.empty())
        return;
    STATS_KD::_reset();

    // Build kd-tree for accelerator
    nextFreeNode = nAllocedNodes = 0;
    if (maxDepth <= 0)
        maxDepth = std::round(8.0f + 1.3f * Log2Int(int64_t(primitives.size())));

    // Compute bounds for kd-tree construction
    std::vector<AxisAlignedBoundingBox> primBounds;
    primBounds.reserve(primitives.size());
    for (const std::shared_ptr<Primitive> &prim : primitives) {
        AxisAlignedBoundingBox b = prim->sh.bb;
        bounds = AxisAlignedBoundingBox::Union(bounds, b);
        primBounds.push_back(b);
    }

    // Allocate working memory for kd-tree construction
    std::unique_ptr<BoundEdge[]> edges[3];
    for (int i = 0; i < 3; ++i)
        edges[i] = std::make_unique<BoundEdge[]>(2 * primitives.size());
    std::unique_ptr<int[]> prims0(new int[primitives.size()]);
    std::unique_ptr<int[]> prims1(new int[(maxDepth + 1) * primitives.size()]);

    // Initialize _primNums_ for kd-tree construction
    std::unique_ptr<int[]> primNums(new int[primitives.size()]);
    for (size_t i = 0; i < primitives.size(); ++i) {
        primNums[i] = i;
    }

    std::vector<IntersectCount>(0).swap(ints);

    bool withBattery = (splitMethod == KdTreeAccel::SplitMethod::TestSplit) || (splitMethod == KdTreeAccel::SplitMethod::HybridSplit);
    bool withSmartBattery = withBattery;

    if(withBattery) {
        printf("--- KD with HitBattery ---\n");
        printf(" Battery size: %zu\n", battery.size());
        // if battery is too large, only use a random sample

        std::vector<TestRay>* batter_ptr = const_cast<std::vector<TestRay> *>(&battery);
        if(battery.size() > HITCACHESAMPLE) {
            batter_ptr = new std::vector<TestRay>;
            std::sample(battery.begin(), battery.end(), std::back_inserter(*batter_ptr),
                        HITCACHESAMPLE, std::mt19937{std::random_device{}()});
            printf(" Sample size: %zu\n", batter_ptr->size());
        }
        std::vector<TestRayLoc> indices;
        indices.reserve(batter_ptr->size());
        for(int i = 0; i < batter_ptr->size(); ++i)
            indices.emplace_back(i, 0, 1.0e99);

        double costLimit = 1.0e99;
        // Start recursive construction of kd-tree
        buildTree(0, bounds, primBounds, primNums.get(), primitives.size(),
                  maxDepth, edges, prims0.get(), prims1.get(), 0, *batter_ptr, indices, -1, 0.0, 1.0e99,
                  probabilities, costLimit);

        if(battery.size() > HITCACHESAMPLE)
            delete batter_ptr;
    }
    else if(withSmartBattery){
        std::unique_ptr<std::vector<RaySegment>> rays(new std::vector<RaySegment>(battery.size()));
        for(int r = 0; r < battery.size(); ++r){
            rays[r].ray = &(battery[r]);
        }
        std::unique_ptr<RayBoundary[]> stack_x(new RayBoundary[battery.size() * 2]);
        std::unique_ptr<RayBoundary[]> stack_y(new RayBoundary[battery.size() * 2]);
        std::unique_ptr<RayBoundary[]> stack_z(new RayBoundary[battery.size() * 2]);

        BoundaryStack boundaryStack{};
        double costLimit = 1.0e99;
        // Start recursive construction of kd-tree
        buildTreeRDH(0, bounds, primBounds, primNums.get(), primitives.size(),
                     maxDepth, edges, prims0.get(), prims1.get(), 0, rays, stack_x, stack_y, stack_z,
                     -1, 0.0, 1.0e99, probabilities, costLimit, boundaryStack);
    }
    else{
        buildTree(0, bounds, primBounds, primNums.get(), primitives.size(),
                  maxDepth, edges, prims0.get(), prims1.get(), 0, probabilities, -1);
    }

    for(int nodeNum = 0; nodeNum < STATS_KD::interiorNodes + STATS_KD::leafNodes; ++nodeNum){
        ints[nodeNum].level = maxDepth - ints[nodeNum].level;
    }

    PrintTreeInfo();
}

void KdAccelNode::InitLeaf(int *primNums, int np,
                           std::vector<int> *primitiveIndices) {
    flags = 3;
    nPrims |= (np << 2);
    // Store primitive ids for leaf node
    if (np == 0)
        onePrimitive = 0;
    else if (np == 1)
        onePrimitive = primNums[0];
    else {
        primitiveIndicesOffset = primitiveIndices->size();
        for (int i = 0; i < np; ++i) primitiveIndices->push_back(primNums[i]);
    }
    STATS_KD::leafNodes++;
    STATS_KD::totalPrimitives += np;
}

KdTreeAccel::~KdTreeAccel() {
    if (nodes) {
        delete[] nodes;
        nodes = nullptr;
    }
}

std::tuple<double, int, int>KdTreeAccel::SplitSAH(int axis, const AxisAlignedBoundingBox &nodeBounds,
                                     const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums,
                                     int nPrimitives, const std::unique_ptr<BoundEdge[]> edges[3]) {

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;
    double oldCost = isectCost * double(nPrimitives);
    double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;

    // Initialize edges for _axis_
    for (int i = 0; i < nPrimitives; ++i) {
        int pn = primNums[i];
        const AxisAlignedBoundingBox &apBounds = allPrimBounds[pn];
        edges[axis][2 * i] = BoundEdge(apBounds.min[axis], pn, true);
        edges[axis][2 * i + 1] = BoundEdge(apBounds.max[axis], pn, false);
    }

    // Sort _edges_ for _axis_
    std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
              [](const BoundEdge &e0, const BoundEdge &e1) -> bool {
                  if (e0.t == e1.t)
                      return (int) e0.type < (int) e1.type;
                  else
                      return e0.t < e1.t;
              });

    // Compute cost of all splits for _axis_ to find best
    int nBelow = 0, nAbove = nPrimitives;

    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) {
            --nAbove;
        }
        double edgeT = edges[axis][i].t;
        if (edgeT > nodeBounds.min[axis] && edgeT < nodeBounds.max[axis]) {
            // Compute cost for split at _i_th edge

            // Compute child surface areas for split at _edgeT_
            int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
            double belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                  (edgeT - nodeBounds.min[axis]) *
                                  (d[otherAxis0] + d[otherAxis1]));
            double aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                  (nodeBounds.max[axis] - edgeT) *
                                  (d[otherAxis0] + d[otherAxis1]));
            double pBelow = belowSA * invTotalSA;
            double pAbove = aboveSA * invTotalSA;
            double eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;

            double cost =
                    traversalCost +
                    isectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);

            // Update best split if this is lowest cost so far
            if (cost < bestCost) {
                bestCost = cost;
                bestAxis = axis;
                bestOffset = i;
            }
        }
        if (edges[axis][i].type == EdgeType::Start) {
            ++nBelow;
        }
    }

    //CHECK(nBelow == nPrimitives && nAbove == 0);
    return {bestCost, bestAxis, bestOffset};
};

std::tuple<double, int, int>KdTreeAccel::SplitProb(int axis, const AxisAlignedBoundingBox &nodeBounds,
                                     const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                                     const std::unique_ptr<BoundEdge[]> edges[3],
                                     const std::vector<double> &probabilities) {

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;
    double oldCost = isectCost * double(nPrimitives);
    double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;

    // Initialize edges for _axis_
    for (int i = 0; i < nPrimitives; ++i) {
        int pn = primNums[i];
        const AxisAlignedBoundingBox &apBounds = allPrimBounds[pn];
        edges[axis][2 * i] = BoundEdge(apBounds.min[axis], pn, true);
        edges[axis][2 * i + 1] = BoundEdge(apBounds.max[axis], pn, false);
    }

    // Sort _edges_ for _axis_
    std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
              [](const BoundEdge &e0, const BoundEdge &e1) -> bool {
                  if (e0.t == e1.t)
                      return (int) e0.type < (int) e1.type;
                  else
                      return e0.t < e1.t;
              });

    // Compute cost of all splits for _axis_ to find best
    int nBelow = 0, nAbove = nPrimitives;
    // w/ probability
    double probBelow = 0.0;
    double probAbove = 0.0;

    // add probability for all edges going in
    std::set<int> edgeIn;
    if (!probabilities.empty()) {
        for (int i = 0; i < 2 * nPrimitives; ++i) {
            auto inside = edgeIn.find(edges[axis][i].primNum);
            if (inside != edgeIn.end()) {
                //probAbove += edges[axis][i].type == EdgeType::End ? probabilities[edges[axis][i].primNum] : 0.0;
                probAbove += probabilities[edges[axis][i].primNum];
                edgeIn.erase(inside);
            } else {
                edgeIn.insert(edges[axis][i].primNum);
            }
        }
    }

    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) {
            --nAbove;
            if (!probabilities.empty()) {
                auto inside = edgeIn.find(edges[axis][i].primNum);
                if (inside == edgeIn.end())
                    probAbove -= probabilities[edges[axis][i].primNum];
            }
        }
        double edgeT = edges[axis][i].t;
        if (edgeT > nodeBounds.min[axis] && edgeT < nodeBounds.max[axis]) {
            // Compute cost for split at _i_th edge

            // Compute child surface areas for split at _edgeT_
            int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
            double belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                  (edgeT - nodeBounds.min[axis]) *
                                  (d[otherAxis0] + d[otherAxis1]));
            double aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                  (nodeBounds.max[axis] - edgeT) *
                                  (d[otherAxis0] + d[otherAxis1]));
            double pBelow = belowSA * invTotalSA;
            double pAbove = aboveSA * invTotalSA;
            double eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;

            const double weightSA = 0.0;
            double pca = (probAbove + probBelow > 0) ? weightSA + (2.0 - weightSA) * probAbove / (probAbove + probBelow)
                                                     : 1.0;
            double pcb = (probAbove + probBelow > 0) ? weightSA + (2.0 - weightSA) * probBelow / (probAbove + probBelow)
                                                     : 1.0;

            double cost =
                    traversalCost +
                    isectCost * (1 - eb) * (pcb * pBelow * nBelow + pca * pAbove * nAbove);

            // Update best split if this is lowest cost so far
            if (cost < bestCost) {
                bestCost = cost;
                bestAxis = axis;
                bestOffset = i;
            }
        }
        if (edges[axis][i].type == EdgeType::Start) {
            ++nBelow;
            if (!probabilities.empty()) {
                auto inside = edgeIn.find(edges[axis][i].primNum);
                if (inside == edgeIn.end())
                    probBelow += probabilities[edges[axis][i].primNum];
            }
        }
    }

    //CHECK(nBelow == nPrimitives && nAbove == 0);
    return {bestCost, bestAxis, bestOffset};
};

std::tuple<double, int, int>KdTreeAccel::SplitTest(int axis, const AxisAlignedBoundingBox &nodeBounds,
                                                   const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums,
                                                   int nPrimitives, const std::unique_ptr<BoundEdge[]> edges[3],
                                                   const std::vector<TestRay> &battery, const std::vector<TestRayLoc> &local_battery,
                                                   const std::vector<double> &primChance, double tMax) {

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;
    double oldCost = isectCost * double(nPrimitives);

    // Initialize edges for _axis_
    for (int i = 0; i < nPrimitives; ++i) {
        int pn = primNums[i];
        const AxisAlignedBoundingBox &apBounds = allPrimBounds[pn];
        edges[axis][2 * i] = BoundEdge(apBounds.min[axis], pn, true);
        edges[axis][2 * i + 1] = BoundEdge(apBounds.max[axis], pn, false);
    }

    double probBelow = 0.0;
    double probAbove = 0.0;
    for (int i = 0; i < 2 * nPrimitives; ++i) {
        /*if (edges[axis][i].type == EdgeType::Start)
            probAbove = edges[axis][i].primNum;*/
        if (edges[axis][i].type == EdgeType::End)
            probAbove += primChance[edges[axis][i].primNum];
    }
    // normalise to [0..1]
    double probMax = probAbove;

    // Sort _edges_ for _axis_
    std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
              [](const BoundEdge &e0, const BoundEdge &e1) -> bool {
                  if (e0.t == e1.t)
                      return (int) e0.type < (int) e1.type;
                  else
                      return e0.t < e1.t;
              });

    // Compute cost of all splits for _axis_ to find best
    int nBelow = 0, nAbove = nPrimitives;
    double inv_denum = 1.0 / (double) local_battery.size();

    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) {
            --nAbove;
            probAbove -= primChance[edges[axis][i].primNum];
        }
        double edgeT = edges[axis][i].t;
        if (edgeT > nodeBounds.min[axis] && edgeT < nodeBounds.max[axis]) {
            // Compute cost for split at _i_th edge
            double eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;

            {//if(local_battery.size() >= HITCACHEMIN) {
                int hitCountB = 0;
                int hitCountA = 0;
                int hitCountBoth = 0;

                auto ray = Ray();
                double locTMax = tMax;
#pragma omp parallel for default(none) firstprivate(ray) shared(battery, axis, hitCountA, hitCountB, hitCountBoth, local_battery, edges, bestAxis, bestOffset, edgeT)
                for (auto &ind: local_battery) {
                    ray.origin = battery[ind.index].pos;
                    ray.direction = battery[ind.index].dir;
                    Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
                    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
                    /*hitcount1 += b1.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;
                    hitcount0 += b0.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;*/

                    // Compute parametric distance along ray to split plane
                    double tSplit = edgeT;
                    //int axis = node->SplitAxis();
                    double tPlane = (tSplit - ray.origin[axis]) * invDir[axis];

                    // Get node children pointers for ray
                    const KdAccelNode *firstChild, *secondChild;
                    int belowFirst =
                            (ray.origin[axis] < tSplit) ||
                            (ray.origin[axis] == tSplit && ray.direction[axis] <= 0);

                    // Advance to next child node, possibly enqueue other child
                    if (tPlane > ind.tMax || tPlane <= 0)
#pragma omp atomic
                        hitCountA++;// test first child
                    else if (tPlane < ind.tMin)
#pragma omp atomic
                        hitCountB++;// test second child
                    else {
#pragma omp atomic
                        hitCountBoth++;
                    }
                }
                double pAbove = (double) (hitCountA + hitCountBoth) * inv_denum;
                double pBelow = (double) (hitCountB + hitCountBoth) * inv_denum;
                double pBoth = (double) (hitCountBoth) * inv_denum;

                /*if(hitCountB-hitCountA < 0) {
                    pAbove = (double) (std::abs(hitCountB - hitCountA)) / (double) (hitCountA + hitCountB);
                    pBelow = 1.0 - pAbove;
                }
                else {
                    pBelow = (double) (std::abs(hitCountA - hitCountB)) / (double) (hitCountA + hitCountB);
                    pAbove = 1.0 - pBelow;
                }*/

                double cost =
                        traversalCost
                        + isectCost * (1 - eb) *
                          (/*pBelow **/ probBelow / probMax * nBelow + /*pAbove **/ probAbove / probMax * nAbove);
                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = axis;
                    bestOffset = i;
                }
            }
        }
        if (edges[axis][i].type == EdgeType::Start) {
            ++nBelow;
            probBelow += primChance[edges[axis][i].primNum];
        }
    }
    //CHECK(nBelow == nPrimitives && nAbove == 0);

    return {bestCost, bestAxis, bestOffset};
};

std::tuple<double, int, int> KdTreeAccel::SplitHybrid(int axis, const AxisAlignedBoundingBox &nodeBounds,
                                                      const std::vector<AxisAlignedBoundingBox> &allPrimBounds,
                                                      int *primNums, int nPrimitives,
                                                      const std::unique_ptr<BoundEdge[]> edges[3],
                                                      const std::vector<TestRay> &battery,
                                                      const std::vector<TestRayLoc> &local_battery,
                                                      const std::vector<double> &primChance, double tMax) const {

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;
    double oldCost = isectCost * double(nPrimitives);
    double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;

    // Initialize edges for _axis_
    for (int i = 0; i < nPrimitives; ++i) {
        int pn = primNums[i];
        const AxisAlignedBoundingBox &apBounds = allPrimBounds[pn];
        edges[axis][2 * i] = BoundEdge(apBounds.min[axis], pn, true);
        edges[axis][2 * i + 1] = BoundEdge(apBounds.max[axis], pn, false);
    }

    double probBelow = 0.0;
    double probAbove = 0.0;
    for (int i = 0; i < 2 * nPrimitives; ++i) {
        /*if (edges[axis][i].type == EdgeType::Start)
            probAbove = edges[axis][i].primNum;*/
        if (edges[axis][i].type == EdgeType::End)
            probAbove += primChance[edges[axis][i].primNum];
    }
    // normalise to [0..1]
    double probMax = probAbove;
    //probAbove = 1.0;

    // Sort _edges_ for _axis_
    std::sort(&edges[axis][0], &edges[axis][2 * nPrimitives],
              [](const BoundEdge &e0, const BoundEdge &e1) -> bool {
                  if (e0.t == e1.t)
                      return (int) e0.type < (int) e1.type;
                  else
                      return e0.t < e1.t;
              });

    // Compute cost of all splits for _axis_ to find best
    int nBelow = 0, nAbove = nPrimitives;
    double inv_denum = 1.0 / local_battery.size();

    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) {
            --nAbove;
            probAbove -= primChance[edges[axis][i].primNum];
        }
        double edgeT = edges[axis][i].t;
        if (edgeT > nodeBounds.min[axis] && edgeT < nodeBounds.max[axis]) {
            // Compute cost for split at _i_th edge

            // Compute child surface areas for split at _edgeT_
            int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
            double belowSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                  (edgeT - nodeBounds.min[axis]) *
                                  (d[otherAxis0] + d[otherAxis1]));
            double aboveSA = 2 * (d[otherAxis0] * d[otherAxis1] +
                                  (nodeBounds.max[axis] - edgeT) *
                                  (d[otherAxis0] + d[otherAxis1]));
            double pBelow = belowSA * invTotalSA;
            double pAbove = aboveSA * invTotalSA;
            double eb = (nAbove == 0 || nBelow == 0) ? emptyBonus : 0;

            double cost_sah =
                    traversalCost +
                    isectCost * (1 - eb) * (pBelow * nBelow + pAbove * nAbove);

            {//if(local_battery.size() >= HITCACHEMIN) {
                int hitCountB = 0;
                int hitCountA = 0;
                int hitCountBoth = 0;
                double pBoth = 0.0;

                auto ray = Ray();
                double locTMax = tMax;
#pragma omp parallel for default(none) firstprivate(ray) shared(battery, axis, hitCountA, hitCountB, hitCountBoth, local_battery, edges, bestAxis, bestOffset, edgeT)
                for (auto &ind: local_battery) {
                    ray.origin = battery[ind.index].pos;
                    ray.direction = battery[ind.index].dir;
                    Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
                    int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
                    /*hitcount1 += b1.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;
                    hitcount0 += b0.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;*/

                    // Compute parametric distance along ray to split plane
                    double tSplit = edgeT;
                    //int axis = node->SplitAxis();
                    double tPlane = (tSplit - ray.origin[axis]) * invDir[axis];

                    // Get node children pointers for ray
                    const KdAccelNode *firstChild, *secondChild;
                    int belowFirst =
                            (ray.origin[axis] < tSplit) ||
                            (ray.origin[axis] == tSplit && ray.direction[axis] <= 0);

                    // Advance to next child node, possibly enqueue other child
                    if (tPlane > ind.tMax || tPlane <= 0)
#pragma omp atomic
                        hitCountA++;// test first child
                    else if (tPlane < ind.tMin)
#pragma omp atomic
                        hitCountB++;// test second child
                    else {
#pragma omp atomic
                        hitCountBoth++;
                    }
                }
                pAbove = (double) (hitCountA + hitCountBoth) * inv_denum;
                pBelow = (double) (hitCountB + hitCountBoth) * inv_denum;
                pBoth = (double) (hitCountBoth) * inv_denum;

                /*if(hitCountB-hitCountA < 0) {
                    pAbove = (double) (std::abs(hitCountB - hitCountA)) / (double) (hitCountA + hitCountB);
                    pBelow = 1.0 - pAbove;
                }
                else {
                    pBelow = (double) (std::abs(hitCountA - hitCountB)) / (double) (hitCountA + hitCountB);
                    pAbove = 1.0 - pBelow;
                }*/

                double cost =
                        traversalCost
                        + isectCost * (1 - eb) *
                          (/*pBelow **/ probBelow / probMax * nBelow + /*pAbove **/ probAbove / probMax * nAbove);

                double alpha = 0.1;
                double beta = 0.9;
                double weight = alpha * (1.0 - (1.0 / (1.0 + beta * (double)local_battery.size())));
                double linCost = weight * cost + (1.0 - weight) * cost_sah;

                if (linCost < bestCost) {
                    bestCost = linCost;
                    bestAxis = axis;
                    bestOffset = i;
                }
            }
        }
        if (edges[axis][i].type == EdgeType::Start) {
            ++nBelow;
            probBelow += primChance[edges[axis][i].primNum];
        }
    }
    //CHECK(nBelow == nPrimitives && nAbove == 0);

    return {bestCost, bestAxis, bestOffset};
};

void KdTreeAccel::buildTree(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                            int depth, const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1,
                            int badRefines, const std::vector<double> &probabilities, int prevSplitAxis) {
    //CHECK_EQ(nodeNum, nextFreeNode);
    assert(nodeNum == nextFreeNode);
    // Get next free node from _nodes_ array
    if (nextFreeNode == nAllocedNodes) {
        int nNewAllocNodes = std::max(2 * nAllocedNodes, 512);
        KdAccelNode *n = new KdAccelNode[nNewAllocNodes];
        if (nAllocedNodes > 0) {
            memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
            delete[] nodes;
        }
        nodes = n;
        nAllocedNodes = nNewAllocNodes;
    }
    ++nextFreeNode;

    nodes[nodeNum].nodeId = nodeNum;
    ints.emplace_back();
    auto& intstat = ints.back(); // ints[nodeNum]
    intstat.nbPrim = nPrimitives;
    intstat.level = depth;

    // Initialize leaf node if termination criteria met
    if (nPrimitives <= maxPrims || depth == 0) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Initialize interior node and continue recursion

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;
    /*double oldCost = isectCost * double(nPrimitives);
    double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;*/

    // Choose which axis to split along
    int axis = nodeBounds.MaximumExtent();
    int retries = 0;
    retrySplit:

    if(splitMethod == SplitMethod::ProbSplit && !probabilities.empty())
        std::tie(bestCost, bestAxis, bestOffset) = SplitProb(axis, nodeBounds, allPrimBounds, primNums, nPrimitives, edges, probabilities);
    else if(splitMethod == SplitMethod::SAH)
        std::tie(bestCost, bestAxis, bestOffset) = SplitSAH(axis, nodeBounds, allPrimBounds, primNums, nPrimitives, edges);

    // Create leaf if no good splits were found
    if (bestAxis == -1 && retries < 2) {
        ++retries;
        if (prevSplitAxis >= 0) {
            axis = (axis + 1) % 3;
            if (retries == 1 && axis == prevSplitAxis) {
                axis = (axis + 1) % 3;
            } else if (retries == 2) {
                axis = prevSplitAxis;
            }
        } else
            axis = (axis + 1) % 3;
        goto retrySplit;
    }

    double oldCost = isectCost * double(nPrimitives);
    if (bestCost > oldCost) ++badRefines;
    if ((bestCost > 4 * oldCost && nPrimitives < 16) || bestAxis == -1 ||
        badRefines == 3) {
        if(bestCost > 4 * oldCost) STATS_KD::leafHigherCost++;
        if(badRefines == 3) STATS_KD::leafBadRefine++;
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Classify primitives with respect to split
    int n0 = 0, n1 = 0;
    for (int i = 0; i < bestOffset; ++i)
        if (edges[bestAxis][i].type == EdgeType::Start)
            prims0[n0++] = edges[bestAxis][i].primNum;
    for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
        if (edges[bestAxis][i].type == EdgeType::End)
            prims1[n1++] = edges[bestAxis][i].primNum;

    STATS_KD::splitSAH++;

    // Recursively initialize children nodes
    double tSplit = edges[bestAxis][bestOffset].t;
    AxisAlignedBoundingBox bounds0 = nodeBounds, bounds1 = nodeBounds;
    bounds0.max[bestAxis] = bounds1.min[bestAxis] = tSplit;

    buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines, probabilities, bestAxis);

    int aboveChild = nextFreeNode;
    nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
    buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines, probabilities, bestAxis);
}

// default construction algorithm for RDH
void KdTreeAccel::buildTree(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                            int depth, const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1,
                            int badRefines, const std::vector<TestRay> &battery,
                            const std::vector<TestRayLoc> &local_battery, int prevSplitAxis, double tMin, double tMax,
                            const std::vector<double> &primChance, double oldCost) {
    //CHECK_EQ(nodeNum, nextFreeNode);
    assert(nodeNum == nextFreeNode);
    // Get next free node from _nodes_ array
    if (nextFreeNode == nAllocedNodes) {
        int nNewAllocNodes = std::max(2 * nAllocedNodes, 512);
        KdAccelNode *n = new KdAccelNode[nNewAllocNodes];
        if (nAllocedNodes > 0) {
            memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
            delete[] nodes;
        }
        nodes = n;
        nAllocedNodes = nNewAllocNodes;
    }
    ++nextFreeNode;

    nodes[nodeNum].nodeId = nodeNum;
    ints.emplace_back();
    auto& intstat = ints.back(); // ints[nodeNum]
    intstat.nbPrim = nPrimitives;
    intstat.level = depth;

    // Initialize leaf node if termination criteria met
    if (nPrimitives <= maxPrims || depth == 0) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Initialize interior node and continue recursion

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;

    //double oldCost = isectCost * double(nPrimitives);
    /*double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;*/

    // Choose which axis to split along
    int axis = nodeBounds.MaximumExtent();
    int retries = 0;

    retrySplit:
    if(splitMethod == SplitMethod::TestSplit && !local_battery.empty())
        std::tie(bestCost, bestAxis, bestOffset) = SplitTest(axis, nodeBounds, allPrimBounds, primNums, nPrimitives, edges, battery, local_battery, primChance, tMax);
    else if(splitMethod == SplitMethod::HybridSplit && !local_battery.empty())
        std::tie(bestCost, bestAxis, bestOffset) = SplitHybrid(axis, nodeBounds, allPrimBounds, primNums, nPrimitives,
                                                               edges, battery, local_battery, primChance, tMax);
    else
        std::tie(bestCost, bestAxis, bestOffset) = SplitSAH(axis, nodeBounds, allPrimBounds, primNums, nPrimitives, edges);

    /*if(local_battery.size() < HITCACHEMIN){
            bestCost = bestCostSAH;
            bestAxis = bestAxisSAH;
            bestOffset = bestOffsetSAH;
    }*/
    // Create leaf if no good splits were found
    if (bestAxis == -1 && retries < 2) {
        ++retries;
        if (prevSplitAxis >= 0) {
            axis = (axis + 1) % 3;
            if (retries == 1 && axis == prevSplitAxis) {
                axis = (axis + 1) % 3;
            } else if (retries == 2) {
                axis = prevSplitAxis;
            }
        } else
            axis = (axis + 1) % 3;
        goto retrySplit;
    }

    if (bestCost > oldCost) {
        //++badRefines;
    }

    if ((bestCost > 4.0 * oldCost && nPrimitives < 16) || bestAxis == -1 ||
        badRefines == 3) {
        if(bestCost > 4.0 * oldCost)
            ++STATS_KD::leafHigherCost;
        if(badRefines >= 3)
            ++STATS_KD::leafBadRefine;
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    std::vector<TestRayLoc> battery_below, battery_above;
    {
        auto ray = Ray();
        double locTMax = tMax;
#pragma omp parallel default(none) firstprivate(ray) shared(battery, local_battery, edges, bestAxis, bestOffset, battery_above, battery_below)
        {
            std::vector<TestRayLoc> battery_below_local, battery_above_local;
#pragma omp for
            for (auto ind : local_battery) {
                ray.origin = battery[ind.index].pos;
                ray.direction = battery[ind.index].dir;
                Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
                int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
                /*hitcount1 += b1.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;
                hitcount0 += b0.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;*/

                {
                    // Compute parametric distance along ray to split plane
                    double tSplit = edges[bestAxis][bestOffset].t;
                    //int axis = node->SplitAxis();
                    double tPlane = (tSplit - ray.origin[bestAxis]) * invDir[bestAxis];

                    // Get node children pointers for ray
                    int belowFirst =
                            (ray.origin[bestAxis] < tSplit) ||
                            (ray.origin[bestAxis] == tSplit && ray.direction[bestAxis] <= 0);

                    // Advance to next child node, possibly enqueue other child
                    {
                        if (tPlane > ind.tMax|| tPlane <= 0) {
                            battery_above_local.emplace_back(ind.index, ind.tMin, ind.tMax);
                        } else if (tPlane < ind.tMin) {
                            battery_below_local.emplace_back(ind.index, ind.tMin, ind.tMax);
                        } else {
                            battery_above_local.emplace_back(ind.index, ind.tMin, tPlane);
                            battery_below_local.emplace_back(ind.index, tPlane, ind.tMax);
                        }
                    }
                }
            }

            // combine local
#pragma omp critical
            {
                battery_above.insert(battery_above.end(), battery_above_local.begin(), battery_above_local.end());
                battery_below.insert(battery_below.end(), battery_below_local.begin(), battery_below_local.end());
            }
        }
    }

    // Classify primitives with respect to split
    int n0 = 0, n1 = 0;
    for (int i = 0; i < bestOffset; ++i)
        if (edges[bestAxis][i].type == EdgeType::Start)
            prims0[n0++] = edges[bestAxis][i].primNum;
    for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
        if (edges[bestAxis][i].type == EdgeType::End)
            prims1[n1++] = edges[bestAxis][i].primNum;

    STATS_KD::splitRay++;

    // Recursively initialize children nodes
    double tSplit = edges[bestAxis][bestOffset].t;
    AxisAlignedBoundingBox bounds0 = nodeBounds, bounds1 = nodeBounds;
    bounds0.max[bestAxis] = bounds1.min[bestAxis] = tSplit;
    if(battery_below.size() < HITCACHEMIN){
        std::vector<double> dummy;
        buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, dummy, bestAxis);
    }
    else {
        buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, battery, battery_below, bestAxis, tMin, tSplit,
                  primChance, bestCost);
    }

    int aboveChild = nextFreeNode;
    nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
    if(battery_above.size() < HITCACHEMIN){
        std::vector<double> dummy;
        buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, dummy, bestAxis);
    }
    else {
        buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, battery, battery_above, bestAxis, tSplit, tMax,
                  primChance, bestCost);
    }
}

// default construction algorithm for RDH
void KdTreeAccel::buildTreeRDH(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                            int depth,
                            const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1, int badRefines,
                            const std::unique_ptr<std::vector<RaySegment>> &battery, std::unique_ptr<RayBoundary[]>& stack_x, std::unique_ptr<RayBoundary[]>& stack_y,
                                    std::unique_ptr<RayBoundary[]>& stack_z, int prevSplitAxis, double tMin, double tMax,
                            const std::vector<double> &primChance, double oldCost, BoundaryStack bound) {
    //CHECK_EQ(nodeNum, nextFreeNode);
    assert(nodeNum == nextFreeNode);
    // Get next free node from _nodes_ array
    if (nextFreeNode == nAllocedNodes) {
        int nNewAllocNodes = std::max(2 * nAllocedNodes, 512);
        KdAccelNode *n = new KdAccelNode[nNewAllocNodes];
        if (nAllocedNodes > 0) {
            memcpy(n, nodes, nAllocedNodes * sizeof(KdAccelNode));
            delete[] nodes;
        }
        nodes = n;
        nAllocedNodes = nNewAllocNodes;
    }
    ++nextFreeNode;

    nodes[nodeNum].nodeId = nodeNum;
    ints.emplace_back();
    auto& intstat = ints.back(); // ints[nodeNum]
    intstat.nbPrim = nPrimitives;
    intstat.level = depth;

    // Initialize leaf node if termination criteria met
    if (nPrimitives <= maxPrims || depth == 0) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Initialize interior node and continue recursion

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;

    //double oldCost = isectCost * double(nPrimitives);
    /*double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;*/

    // Choose which axis to split along
    int axis = nodeBounds.MaximumExtent();
    int retries = 0;

    retrySplit:
    if(splitMethod == SplitMethod::TestSplit && battery)
        std::tie(bestCost, bestAxis, bestOffset) = SplitTest(axis, nodeBounds, allPrimBounds, primNums, nPrimitives, edges, battery, local_battery, primChance, tMax);
    else if(splitMethod == SplitMethod::HybridSplit && battery)
        std::tie(bestCost, bestAxis, bestOffset) = SplitHybrid(axis, nodeBounds, allPrimBounds, primNums, nPrimitives,
                                                               edges, battery, local_battery, primChance, tMax);
    else
        std::tie(bestCost, bestAxis, bestOffset) = SplitSAH(axis, nodeBounds, allPrimBounds, primNums, nPrimitives, edges);

    /*if(local_battery.size() < HITCACHEMIN){
            bestCost = bestCostSAH;
            bestAxis = bestAxisSAH;
            bestOffset = bestOffsetSAH;
    }*/
    // Create leaf if no good splits were found
    if (bestAxis == -1 && retries < 2) {
        ++retries;
        if (prevSplitAxis >= 0) {
            axis = (axis + 1) % 3;
            if (retries == 1 && axis == prevSplitAxis) {
                axis = (axis + 1) % 3;
            } else if (retries == 2) {
                axis = prevSplitAxis;
            }
        } else
            axis = (axis + 1) % 3;
        goto retrySplit;
    }

    if (bestCost > oldCost) {
        //++badRefines;
    }

    if ((bestCost > 4.0 * oldCost && nPrimitives < 16) || bestAxis == -1 ||
        badRefines == 3) {
        if(bestCost > 4.0 * oldCost)
            ++STATS_KD::leafHigherCost;
        if(badRefines >= 3)
            ++STATS_KD::leafBadRefine;
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    std::vector<TestRayLoc> battery_below, battery_above;
    {
        auto ray = Ray();
        double locTMax = tMax;
#pragma omp parallel default(none) firstprivate(ray) shared(battery, local_battery, edges, bestAxis, bestOffset, battery_above, battery_below)
        {
            std::vector<TestRayLoc> battery_below_local, battery_above_local;
#pragma omp for
            for (auto ind : local_battery) {
                ray.origin = battery[ind.index].pos;
                ray.direction = battery[ind.index].dir;
                Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
                int dirIsNeg[3] = {invDir.x < 0, invDir.y < 0, invDir.z < 0};
                /*hitcount1 += b1.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;
                hitcount0 += b0.IntersectBox(ray, invDir, dirIsNeg) ? 1 : 0;*/

                {
                    // Compute parametric distance along ray to split plane
                    double tSplit = edges[bestAxis][bestOffset].t;
                    //int axis = node->SplitAxis();
                    double tPlane = (tSplit - ray.origin[bestAxis]) * invDir[bestAxis];

                    // Get node children pointers for ray
                    int belowFirst =
                            (ray.origin[bestAxis] < tSplit) ||
                            (ray.origin[bestAxis] == tSplit && ray.direction[bestAxis] <= 0);

                    // Advance to next child node, possibly enqueue other child
                    {
                        if (tPlane > ind.tMax|| tPlane <= 0) {
                            battery_above_local.emplace_back(ind.index, ind.tMin, ind.tMax);
                        } else if (tPlane < ind.tMin) {
                            battery_below_local.emplace_back(ind.index, ind.tMin, ind.tMax);
                        } else {
                            battery_above_local.emplace_back(ind.index, ind.tMin, tPlane);
                            battery_below_local.emplace_back(ind.index, tPlane, ind.tMax);
                        }
                    }
                }
            }

            // combine local
#pragma omp critical
            {
                battery_above.insert(battery_above.end(), battery_above_local.begin(), battery_above_local.end());
                battery_below.insert(battery_below.end(), battery_below_local.begin(), battery_below_local.end());
            }
        }
    }

    // Classify primitives with respect to split
    int n0 = 0, n1 = 0;
    for (int i = 0; i < bestOffset; ++i)
        if (edges[bestAxis][i].type == EdgeType::Start)
            prims0[n0++] = edges[bestAxis][i].primNum;
    for (int i = bestOffset + 1; i < 2 * nPrimitives; ++i)
        if (edges[bestAxis][i].type == EdgeType::End)
            prims1[n1++] = edges[bestAxis][i].primNum;

    STATS_KD::splitRay++;

    // Recursively initialize children nodes
    double tSplit = edges[bestAxis][bestOffset].t;
    AxisAlignedBoundingBox bounds0 = nodeBounds, bounds1 = nodeBounds;
    bounds0.max[bestAxis] = bounds1.min[bestAxis] = tSplit;


    /* --- RDH ---
     * When a splitting plane position is decided and a left subtree should be constructed,
     * we check the boundaries in the corresponding axis and
     * mark all ray segments that have to be shortened (are cut by the splitting plane).
     * The original ray boundaries are stored in the stack of
     * signed distances associated with each ray segment data structure.
     * For the marked rays we recompute the boundaries in a corresponding axis.
     * All ray boundaries are then resorted by quicksort before we start to evaluate the RDH.
     */
    RayBoundary* to_sort;
    // 1. select by axis
    if(bestAxis == 0) {
        to_sort = stack_x.get();
    }
    else if (axis == 1){
        to_sort = stack_y.get();
    }
    else {
        to_sort = stack_z.get();
    }
    // 2. mark rays depending on boundaries
    for(int i = 0; i < battery.size(); ++i) {
        auto& segment = to_sort[i];
        // 3. store original boundaries
        segment.rs->distances.emplace_back(Vector2d{segment.rs->tMin, segment.rs->tMax});
        if(segment.rs->tMin < tSplit || tSplit < segment.rs->tMax){
            // 4. mark segment
            // TODO:
            segment.rs->tMin = std::max(segment.rs->tMin, tSplit);
            segment.rs->tMax = std::min(segment.rs->tMax, tSplit);
        }
    }
    if(battery_below.size() < HITCACHEMIN){
        std::vector<double> dummy;
        buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, dummy, bestAxis);
    }
    else {
        buildTreeRDH(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, battery, stack_x, stack_y, stack_z, bestAxis, tMin, tSplit,
                  primChance, bestCost, bound);
    }

    int aboveChild = nextFreeNode;
    nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
    if(battery_above.size() < HITCACHEMIN){
        std::vector<double> dummy;
        buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, dummy, bestAxis);
    }
    else {
        buildTreeRDH(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
                  prims0, prims1 + nPrimitives, badRefines, battery, stack_x, stack_y, stack_z, bestAxis, tSplit, tMax,
                  primChance, bestCost, bound);
    }
}

bool KdTreeAccel::Intersect(Ray &ray) {
    //ProfilePhase p(Prof::AccelIntersect);
    // Compute initial parametric range of ray inside kd-tree extent
    double tMin = 0.0, tMax = 1.0e99;

    // Neglect initial parametric test, since a ray starts always from inside
    /*if (!bounds.IntersectP(ray, &tMin, &tMax)) {
        return false;
    }*/

    // Prepare to traverse kd-tree for ray
    Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
    constexpr int maxTodo = 64;
    KdToDo todo[maxTodo];
    int todoPos = 0;

    // Traverse kd-tree nodes in order for ray
    bool hit = false;
    const KdAccelNode *node = &nodes[0];
    while (node != nullptr) {
        // Bail out if we found a hit closer than the current node
        if (ray.tMax < tMin) break;
        if (!node->IsLeaf()) {
            // Process kd-tree interior node

            // Compute parametric distance along ray to split plane
            int axis = node->SplitAxis();
            double tPlane = (node->SplitPos() - ray.origin[axis]) * invDir[axis];

            // Get node children pointers for ray
            const KdAccelNode *firstChild, *secondChild;
            int belowFirst =
                    (ray.origin[axis] < node->SplitPos()) ||
                    (ray.origin[axis] == node->SplitPos() && ray.direction[axis] <= 0);
            if (belowFirst) {
                firstChild = node + 1;
                secondChild = &nodes[node->AboveChild()];
            } else {
                firstChild = &nodes[node->AboveChild()];
                secondChild = node + 1;
            }

            // Advance to next child node, possibly enqueue other child
            if (tPlane > tMax || tPlane <= 0)
                node = firstChild;
            else if (tPlane < tMin)
                node = secondChild;
            else {
                // Enqueue _secondChild_ in KDtodo list
                todo[todoPos].node = secondChild;
                todo[todoPos].tMin = tPlane;
                todo[todoPos].tMax = tMax;
                ++todoPos;
                node = firstChild;
                tMax = tPlane;
            }
        } else {
            // Check for intersections inside leaf node
            int nPrimitives = node->nPrimitives();
            if (nPrimitives == 1) {
                const std::shared_ptr<Primitive> &p =
                        primitives[node->onePrimitive];

                // Check one primitive inside leaf node
                if (p->globalId != ray.lastIntersected && p->Intersect(ray))
                    hit = true;
            } else {
                for (int i = 0; i < nPrimitives; ++i) {
                    int index =
                            primitiveIndices[node->primitiveIndicesOffset + i];
                    const std::shared_ptr<Primitive> &p = primitives[index];
                    // Check one primitive inside leaf node
                    if (p->globalId != ray.lastIntersected && p->Intersect(ray))
                        hit = true;
                }
            }

            // Grab next node to process from kdtodo list
            if (todoPos > 0) {
                --todoPos;
                node = todo[todoPos].node;
                tMin = todo[todoPos].tMin;
                tMax = todo[todoPos].tMax;
            } else
                break;
        }
    }

    return hit;
}

bool KdTreeAccel::IntersectStat(RayStat &ray) {
    //ProfilePhase p(Prof::AccelIntersect);
    // Compute initial parametric range of ray inside kd-tree extent
    double tMin = 0.0, tMax = 1.0e99;

    // Neglect initial parametric test, since a ray starts always from inside
    /*if (!bounds.IntersectP(ray, &tMin, &tMax)) {
        return false;
    }*/

    // Prepare to traverse kd-tree for ray
    Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
    constexpr int maxTodo = 64;
    KdToDo todo[maxTodo];
    int todoPos = 0;

    // Traverse kd-tree nodes in order for ray
    bool hit = false;
    const KdAccelNode *node = &nodes[0];
    while (node != nullptr) {

        ints[node->nodeId].nbChecks+=ray.traversalSteps;

        // Bail out if we found a hit closer than the current node
        if (ray.tMax < tMin) break;
        if (!node->IsLeaf()) {
            // Process kd-tree interior node
            ray.traversalSteps++;
            ints[node->nodeId].nbIntersects++;

            // Compute parametric distance along ray to split plane
            int axis = node->SplitAxis();
            double tPlane = (node->SplitPos() - ray.origin[axis]) * invDir[axis];

            // Get node children pointers for ray
            const KdAccelNode *firstChild, *secondChild;
            int belowFirst =
                    (ray.origin[axis] < node->SplitPos()) ||
                    (ray.origin[axis] == node->SplitPos() && ray.direction[axis] <= 0);
            if (belowFirst) {
                firstChild = node + 1;
                secondChild = &nodes[node->AboveChild()];
            } else {
                firstChild = &nodes[node->AboveChild()];
                secondChild = node + 1;
            }

            // Advance to next child node, possibly enqueue other child
            if (tPlane > tMax || tPlane <= 0)
                node = firstChild;
            else if (tPlane < tMin)
                node = secondChild;
            else {
                // Enqueue _secondChild_ in KDtodo list
                todo[todoPos].node = secondChild;
                todo[todoPos].tMin = tPlane;
                todo[todoPos].tMax = tMax;
                ++todoPos;
                node = firstChild;
                tMax = tPlane;
            }
        } else {
            // Check for intersections inside leaf node
            int nPrimitives = node->nPrimitives();
            if (nPrimitives == 1) {
                const std::shared_ptr<Primitive> &p =
                        primitives[node->onePrimitive];

                // Check one primitive inside leaf node
                ray.traversalSteps++;
                if (p->globalId != ray.lastIntersected && p->IntersectStat(ray))
                    hit = true;
            } else {
                for (int i = 0; i < nPrimitives; ++i) {
                    int index =
                            primitiveIndices[node->primitiveIndicesOffset + i];
                    const std::shared_ptr<Primitive> &p = primitives[index];
                    // Check one primitive inside leaf node
                    ray.traversalSteps++;
                    if (p->globalId != ray.lastIntersected && p->IntersectStat(ray))
                        hit = true;
                }
            }

            // Grab next node to process from kdtodo list
            if (todoPos > 0) {
                --todoPos;
                node = todo[todoPos].node;
                tMin = todo[todoPos].tMin;
                tMax = todo[todoPos].tMax;
            } else
                break;
        }
    }

    ray.traversalSteps = 0;
    return hit;
}

RTStats KdTreeAccel::IntersectT(Ray &ray) {
    //ProfilePhase p(Prof::AccelIntersect);
    // Compute initial parametric range of ray inside kd-tree extent
    double tMin = 0.0, tMax = 1.0e99;
    /*if (!bounds.IntersectP(ray, &tMin, &tMax)) {
        return false;
    }*/

    // Prepare to traverse kd-tree for ray
    Vector3d invDir(1.0 / ray.direction.x, 1.0 / ray.direction.y, 1.0 / ray.direction.z);
    constexpr int maxTodo = 64;
    KdToDo todo[maxTodo];
    int todoPos = 0;

    // Traverse kd-tree nodes in order for ray
    bool hit = false;
    const KdAccelNode *node = &nodes[0];

    RTStats stat;
    Chronometer time_trav;
    Chronometer time_int;
    while (node != nullptr) {
        // Bail out if we found a hit closer than the current node
        if (ray.tMax < tMin) break;
        if (!node->IsLeaf()) {

            // Process kd-tree interior node
            time_trav.Start();
            stat.nti++;

            // Compute parametric distance along ray to split plane
            int axis = node->SplitAxis();
            double tPlane = (node->SplitPos() - ray.origin[axis]) * invDir[axis];

            // Get node children pointers for ray
            const KdAccelNode *firstChild, *secondChild;
            int belowFirst =
                    (ray.origin[axis] < node->SplitPos()) ||
                    (ray.origin[axis] == node->SplitPos() && ray.direction[axis] <= 0);
            if (belowFirst) {
                firstChild = node + 1;
                secondChild = &nodes[node->AboveChild()];
            } else {
                firstChild = &nodes[node->AboveChild()];
                secondChild = node + 1;
            }

            // Advance to next child node, possibly enqueue other child
            if (tPlane > tMax || tPlane <= 0)
                node = firstChild;
            else if (tPlane < tMin)
                node = secondChild;
            else {
                // Enqueue _secondChild_ in KDtodo list
                todo[todoPos].node = secondChild;
                todo[todoPos].tMin = tPlane;
                todo[todoPos].tMax = tMax;
                ++todoPos;
                node = firstChild;
                tMax = tPlane;
            }
            time_trav.Stop();
        } else {
            time_int.Start();

            // Check for intersections inside leaf node
            int nPrimitives = node->nPrimitives();

            stat.ntl++;
            stat.nit+=nPrimitives;
            if (nPrimitives == 1) {
                const std::shared_ptr<Primitive> &p =
                        primitives[node->onePrimitive];

                // Check one primitive inside leaf node
                if (p->globalId != ray.lastIntersected && p->Intersect(ray))
                    hit = true;
            } else {
                for (int i = 0; i < nPrimitives; ++i) {
                    int index =
                            primitiveIndices[node->primitiveIndicesOffset + i];
                    const std::shared_ptr<Primitive> &p = primitives[index];
                    // Check one primitive inside leaf node
                    if (p->globalId != ray.lastIntersected && p->Intersect(ray))
                        hit = true;
                }
            }

            // Grab next node to process from kdtodo list
            if (todoPos > 0) {
                --todoPos;
                node = todo[todoPos].node;
                tMin = todo[todoPos].tMin;
                tMax = todo[todoPos].tMax;
            } else
                break;
            time_int.Stop();
        }
    }

    stat.timeInt = time_int.ElapsedMs();
    stat.timeTrav = time_trav.ElapsedMs();
    return stat;
}

KdTreeAccel::KdTreeAccel(KdTreeAccel &&src)
noexcept: isectCost(src.isectCost),
          traversalCost(src.traversalCost),
          maxPrims(src.maxPrims),
          emptyBonus(src.emptyBonus),
          primitives(std::move(src.primitives)),
          primitiveIndices(std::move(src.primitiveIndices)) {
    nodes = src.nodes;
    nAllocedNodes = src.nAllocedNodes;
    nextFreeNode = src.nextFreeNode;
    src.nodes = nullptr;
    bb = src.bb;
}

KdTreeAccel::KdTreeAccel(
        const KdTreeAccel &src) noexcept: isectCost(src.isectCost),
                                          traversalCost(src.traversalCost),
                                          maxPrims(src.maxPrims),
                                          emptyBonus(src.emptyBonus),
                                          primitives(src.primitives),
                                          primitiveIndices(src.primitiveIndices) {
    if (nodes)
        exit(44);
}

KdTreeAccel &KdTreeAccel::operator=(const KdTreeAccel &src) noexcept {
    if (nodes)
        exit(44);

    return *this;
}
