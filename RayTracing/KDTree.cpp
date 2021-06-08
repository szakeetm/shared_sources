//
// Created by pascal on 4/21/21.
//

/*
 * KDTree code based on pbrt-v3
 * Copyright (c) 1998-2015, Matt Pharr, Greg Humphreys, and Wenzel Jakob.
 */


#include <BoundingBox.h>
#include <cmath>
#include <set>
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

inline int Log2Int(int32_t v) { return Log2Int((uint32_t)v); }

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

inline int Log2Int(int64_t v) { return Log2Int((uint64_t)v); }

namespace STATS_KD {
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

// KdTreeAccel Local Declarations
struct KdAccelNode {
    // KdAccelNode Methods
    void InitLeaf(int *primNums, int np, std::vector<int> *primitiveIndices);
    void InitInterior(int axis, int ac, double s) {
        split = s;
        flags = axis;
        aboveChild |= (ac << 2);
        STATS_KD::interiorNodes++;
    }
    double SplitPos() const { return split; }
    int nPrimitives() const { return nPrims >> 2; }
    int SplitAxis() const { return flags & 3; }
    bool IsLeaf() const { return (flags & 3) == 3; }
    int AboveChild() const { return aboveChild >> 2; }
    union {
        double split;                 // Interior
        int onePrimitive;            // Leaf
        int primitiveIndicesOffset;  // Leaf
    };

private:
    union {
        int flags;       // Both
        int nPrims;      // Leaf
        int aboveChild;  // Interior
    };
};

enum class EdgeType { Start, End };
struct BoundEdge {
    // BoundEdge Public Methods
    BoundEdge() {}
    BoundEdge(double t, int primNum, bool starting) : t(t), primNum(primNum) {
        type = starting ? EdgeType::Start : EdgeType::End;
    }
    double t;
    int primNum;
    EdgeType type;
};

void KdTreeAccel::ComputeBB() {
    bb = AxisAlignedBoundingBox();
    if(nodes) {
        for(auto prim : primitives){
            bb = AxisAlignedBoundingBox::Union(bb, prim->bb);
        }
    }
}

// KdTreeAccel Method Definitions
KdTreeAccel::KdTreeAccel(std::vector<std::shared_ptr<Primitive>> p, const std::vector<double>& probabilities,
                         int isectCost, int traversalCost, double emptyBonus,
                         int maxPrims, int maxDepth)
        : isectCost(isectCost),
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
        maxDepth = std::round(8 + 1.3f * Log2Int(int64_t(primitives.size())));

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
        edges[i].reset(new BoundEdge[2 * primitives.size()]);
    std::unique_ptr<int[]> prims0(new int[primitives.size()]);
    std::unique_ptr<int[]> prims1(new int[(maxDepth + 1) * primitives.size()]);

    // Initialize _primNums_ for kd-tree construction
    std::unique_ptr<int[]> primNums(new int[primitives.size()]);
    for (size_t i = 0; i < primitives.size(); ++i) primNums[i] = i;

    // Start recursive construction of kd-tree
    buildTree(0, bounds, primBounds, primNums.get(), primitives.size(),
              maxDepth, edges, prims0.get(), prims1.get(), 0, probabilities);

    printf("--- KD STATS ---\n");
    printf(" Total Primitives: %d\n", STATS_KD::totalPrimitives);
    printf(" Total Leaf Nodes: %d\n", STATS_KD::totalLeafNodes);
    printf(" Interior Nodes:   %d\n", STATS_KD::interiorNodes);
    printf(" Leaf Nodes:       %d\n", STATS_KD::leafNodes);

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
    if(nodes) {
        delete[] nodes;
        nodes = nullptr;
    }
}

void KdTreeAccel::buildTree(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds,
                            int *primNums, int nPrimitives, int depth,
                            const std::unique_ptr<BoundEdge[]> edges[3],
                            int *prims0, int *prims1, int badRefines, const std::vector<double>& probabilities) {
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

    // Initialize leaf node if termination criteria met
    if (nPrimitives <= maxPrims || depth == 0) {
        nodes[nodeNum].InitLeaf(primNums, nPrimitives, &primitiveIndices);
        return;
    }

    // Initialize interior node and continue recursion

    // Choose split axis position for interior node
    int bestAxis = -1, bestOffset = -1;
    double bestCost = 1.0e99;
    double oldCost = isectCost * double(nPrimitives);
    double totalSA = nodeBounds.SurfaceArea();
    double invTotalSA = 1.0 / totalSA;
    Vector3d d = nodeBounds.max - nodeBounds.min;

    // Choose which axis to split along
    int axis = nodeBounds.MaximumExtent();
    int retries = 0;
    retrySplit:

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
                      return (int)e0.type < (int)e1.type;
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
    if(!probabilities.empty()) {
        for (int i = 0; i < 2 * nPrimitives; ++i) {
            auto inside = edgeIn.find(edges[axis][i].primNum);
            if(inside != edgeIn.end()) {
                //probAbove += edges[axis][i].type == EdgeType::End ? probabilities[edges[axis][i].primNum] : 0.0;
                probAbove += probabilities[edges[axis][i].primNum];
                edgeIn.erase(inside);
            }
            else {
                edgeIn.insert(edges[axis][i].primNum);
            }
        }
    }

    for (int i = 0; i < 2 * nPrimitives; ++i) {
        if (edges[axis][i].type == EdgeType::End) {
            --nAbove;
            if(!probabilities.empty()) {
                auto inside = edgeIn.find(edges[axis][i].primNum);
                if(inside == edgeIn.end())
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
            double pca = (probAbove + probBelow > 0) ? weightSA + (2.0 - weightSA) * probAbove / (probAbove + probBelow) : 1.0;
            double pcb = (probAbove + probBelow > 0) ? weightSA + (2.0 - weightSA) * probBelow / (probAbove + probBelow) : 1.0;

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
            if(!probabilities.empty()) {
                auto inside = edgeIn.find(edges[axis][i].primNum);
                if(inside == edgeIn.end())
                    probBelow += probabilities[edges[axis][i].primNum];
            }
        }
    }
    //CHECK(nBelow == nPrimitives && nAbove == 0);

    // Create leaf if no good splits were found
    if (bestAxis == -1 && retries < 2) {
        ++retries;
        axis = (axis + 1) % 3;
        goto retrySplit;
    }
    if (bestCost > oldCost) ++badRefines;
    if ((bestCost > 4 * oldCost && nPrimitives < 16) || bestAxis == -1 ||
        badRefines == 3) {
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

    // Recursively initialize children nodes
    double tSplit = edges[bestAxis][bestOffset].t;
    AxisAlignedBoundingBox bounds0 = nodeBounds, bounds1 = nodeBounds;
    bounds0.max[bestAxis] = bounds1.min[bestAxis] = tSplit;
    buildTree(nodeNum + 1, bounds0, allPrimBounds, prims0, n0, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines, probabilities);
    int aboveChild = nextFreeNode;
    nodes[nodeNum].InitInterior(bestAxis, aboveChild, tSplit);
    buildTree(aboveChild, bounds1, allPrimBounds, prims1, n1, depth - 1, edges,
              prims0, prims1 + nPrimitives, badRefines, probabilities);
}

bool KdTreeAccel::Intersect(Ray &ray) {
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

KdTreeAccel::KdTreeAccel(KdTreeAccel &&src) noexcept : isectCost(src.isectCost),
                                                       traversalCost(src.traversalCost),
                                                       maxPrims(src.maxPrims),
                                                       emptyBonus(src.emptyBonus),
                                                       primitives(std::move(src.primitives)),
                                                       primitiveIndices(std::move(src.primitiveIndices)){
    nodes = src.nodes;
    nAllocedNodes = src.nAllocedNodes;
    nextFreeNode = src.nextFreeNode;
    src.nodes = nullptr;
    bb = src.bb;
}

KdTreeAccel::KdTreeAccel(const KdTreeAccel &src) noexcept : isectCost(src.isectCost),
                                                            traversalCost(src.traversalCost),
                                                            maxPrims(src.maxPrims),
                                                            emptyBonus(src.emptyBonus),
                                                            primitives(src.primitives),
                                                            primitiveIndices(src.primitiveIndices) {
    if(nodes)
        exit(44);
}

KdTreeAccel &KdTreeAccel::operator=(const KdTreeAccel &src) noexcept {
    if(nodes)
        exit(44);

    return *this;
}
