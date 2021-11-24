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
const double KD_TREE_EPSILON = 1e-8; // for ropes
enum SplitAxis {
    X_AXIS = 0,
    Y_AXIS = 1,
    Z_AXIS = 2
};

enum AABBFace {
    LEFT = 0,
    RIGHT = 1,
    TOP = 2,
    BOTTOM = 3,
    FRONT = 4,
    BACK = 5
};

// KdTreeAccel Forward Declarations
// KdTreeAccel Local Declarations
struct KdAccelNode {
    // KdAccelNode Methods
    void InitLeaf(int *primNums, int np, std::vector<int> *primitiveIndices);
    void InitInterior(int axis, int ac, double s);
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

    // stats
    int nodeId{0};

    // stackless w/ ropes
    KdAccelNode *parent{nullptr}; // needed if we start not from the root node
    KdAccelNode *ropes[6];
    AxisAlignedBoundingBox bbox;
    [[nodiscard]] const KdAccelNode *getNeighboringNode(const Ray &ray) const;
    [[nodiscard]] const KdAccelNode *getNeighboringNode(const Vector3d &p, const Vector3d &o) const;

    void prettyPrint() const;

private:
    union {
        int flags;       // Both
        int nPrims;      // Leaf
        int aboveChild;  // Interior
    };

    friend class Geometry;

};

struct BoundEdge;

struct RTStats {
    size_t nTraversedInner{0}; // traversed interior nodes
    size_t nTraversedLeaves{0}; // traversed leaves
    size_t nIntersections{0}; // ray-bject intersection
    double timeTrav{0};
    double timeInt{0};
};

struct TestRayLoc {
    TestRayLoc(size_t ind, double min, double max) {
        index = ind;
        tMin = min;
        tMax = max;
    }
    size_t index{0};
    double tMin{0.0};
    double tMax{0.0};
};

struct SplitCandidate {
    SplitCandidate() = default;
    SplitCandidate(double c, int a, int o){
        cost=c;
        axis=a;
        offset=o;
    }
    double cost{1.0e99};
    int axis{-1};
    int offset{-1};
};

struct RopePayload : Payload {
    const KdAccelNode* lastNode;
#if defined(DEBUG)
    Ray lastRay;
#endif
};

class KdTreeAccel : public RTPrimitive {
public:
    using BoundaryFloat = double;
    struct RaySegment{
        BoundaryFloat tMin{-1.0e99};
        BoundaryFloat tMax{1.0e99};
        const TestRay* ray{nullptr};
        std::vector<Vector2d> distances;
    };
    struct RayBoundary{
        RaySegment* rs{nullptr};
        int flag{-1}; // 0: left, 1: on splitting plane, 2: right
    };
    // local stack shows which _RayBoundary/ies from the whole set of boundaries belong to
    //the left, to the right, and to both of them
    struct BoundaryStack{
        BoundaryStack(int ll, int lh, int rl, int rh) :
            leftMin(ll), leftMax(lh), rightMin(rl), rightMax(rh) {};
        int leftMin{-1};
        int leftMax{-1};
        int rightMin{-1};
        int rightMax{-1};
    };

    // KdTreeAccel Public Types
    enum class SplitMethod {
        SAH, ProbSplit, TestSplit, HybridSplit, HybridBin
    };
    friend std::ostream& operator << (std::ostream& os, SplitMethod split_type);

    KdAccelNode *nodes;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<int> primitiveIndices;
public:
    // KdTreeAccel Public Methods
    /*KdTreeAccel(SplitMethod splitMethod, std::vector<std::shared_ptr<Primitive>> p,
                const std::vector<double> &probabilities = std::vector<double>{},
                int isectCost = 80, int traversalCost = 1,
                double emptyBonus = 0.5, int maxPrims = 1, int maxDepth = -1);
    */
    KdTreeAccel(SplitMethod splitMethod, std::vector<std::shared_ptr<Primitive>> p,
                const std::vector<double> &probabilities = std::vector<double>{}, const std::vector<TestRay> &battery = std::vector<TestRay>{},
                int isectCost = 80, int traversalCost = 1,
                double emptyBonus = 0.5, int maxPrims = 1, int maxDepth = -1);
    KdTreeAccel(KdTreeAccel && src) noexcept;
    KdTreeAccel(const KdTreeAccel & src) noexcept;

    KdTreeAccel& operator=(const KdTreeAccel & src) noexcept;
    ~KdTreeAccel() override;

    bool Intersect(Ray &ray) override;
    bool IntersectTrav(Ray &ray);
    RTStats IntersectT(Ray &ray);

    std::vector<IntersectCount> ints;
    bool hasRopes = false;
    bool restartFromNode = false;

    void RemoveRopes();
    void AddRopes(bool optimized);

private:
    void ComputeBB() override;
    // KdTreeAccel Private Methods
    void buildTree(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                   const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                   int depth, const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1,
                   int badRefines, const std::vector<double> &probabilities, int prevSplitAxis);
    void buildTree(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                   const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                   int depth, const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1,
                   int badRefines, const std::vector<TestRay> &battery,
                   const std::vector<TestRayLoc> &local_battery, int prevSplitAxis, double tMin, double tMax,
                   const std::vector<double> &primChance, double oldCost);
    void buildTreeRDH(int nodeNum, const AxisAlignedBoundingBox &nodeBounds,
                      const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                      int depth, const std::unique_ptr<BoundEdge[]> edges[3], int *prims0, int *prims1,
                      int badRefines, const std::unique_ptr<std::vector<RaySegment>> &battery,
                      RayBoundary ** rb_stack, int prevSplitAxis, double tMin,
                      double tMax, const std::vector<double> &primChance, double oldCost, BoundaryStack bound);
    void attachRopes(KdAccelNode *current, KdAccelNode *ropes[], bool optimize);
    void optimizeRopes( KdAccelNode *ropes[], AxisAlignedBoundingBox bbox);

private:
    // KdTreeAccel Private Data
    SplitMethod splitMethod;
    const int isectCost, traversalCost, maxPrims;
    const double emptyBonus;
    int nAllocedNodes, nextFreeNode;
    AxisAlignedBoundingBox bounds;

    std::tuple<double, int, int> SplitSAH(int axis, const AxisAlignedBoundingBox &nodeBounds,
                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums,
                            int nPrimitives, const std::unique_ptr<BoundEdge[]> edges[3]);
    std::tuple<double, int, int> SplitProb(int axis, const AxisAlignedBoundingBox &nodeBounds,
                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds, int *primNums, int nPrimitives,
                            const std::unique_ptr<BoundEdge[]> edges[3],
                            const std::vector<double> &probabilities);

    std::tuple<double, int, int> SplitHybrid(int axis, const AxisAlignedBoundingBox &nodeBounds,
                                             const std::vector<AxisAlignedBoundingBox> &allPrimBounds,
                                             int *primNums, int nPrimitives,
                                             const std::unique_ptr<BoundEdge[]> edges[3],
                                             const std::vector<TestRay> &battery,
                                             const std::vector<TestRayLoc> &local_battery,
                                             const std::vector<double> &primChance, double tMax) const;
    std::tuple<double, int, int, bool, bool> SplitHybridBin(int axis, const AxisAlignedBoundingBox &nodeBounds,
                                                            const std::vector<AxisAlignedBoundingBox> &allPrimBounds,
                                                            int *primNums, int nPrimitives,
                                                            const std::unique_ptr<BoundEdge[]> edges[3],
                                                            const std::vector<TestRay> &battery,
                                                            const std::vector<TestRayLoc> &local_battery,
                                                            const std::vector<double> &primChance, double tMax) const;
    std::tuple<double, int, int>
    SplitTest(int axis, const AxisAlignedBoundingBox &nodeBounds,
              const std::vector<AxisAlignedBoundingBox> &allPrimBounds,
              int *primNums, int nPrimitives,
              const std::unique_ptr<BoundEdge[]> edges[3],
              const std::vector<TestRay> &battery, const std::vector<TestRayLoc> &local_battery,
              const std::vector<double> &primChance, double tMax);

              std::tuple<double, int, int>
    SplitTest(int axis, const AxisAlignedBoundingBox &nodeBounds,
              const std::vector<AxisAlignedBoundingBox> &allPrimBounds,
              int *primNums, int nPrimitives,
              const std::unique_ptr<BoundEdge[]> edges[3],
              const std::unique_ptr<std::vector<RaySegment>> &battery,
              RayBoundary **rb_stack,
              const std::vector<double> &primChance, double tMax,
              const BoundaryStack &boundary_stack);

    friend class Geometry;

    void PrintTreeInfo();

    bool IntersectStat(RayStat &ray) override;
    bool IntersectTravStat(RayStat &ray);

    bool IntersectRope(Ray &ray);
    bool IntersectRopeStat(RayStat &ray);
    bool IntersectRopeRestart(Ray &ray); // to restart from previous target node
    bool IntersectRopeRestartStat(RayStat &ray);

    void addParent(KdAccelNode *current, KdAccelNode *parent);
};

struct KdToDo {
    const KdAccelNode *node;
    double tMin, tMax;
};

#endif //MOLFLOW_PROJ_KDTREE_H
