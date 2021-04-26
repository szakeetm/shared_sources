//
// Created by pascal on 4/21/21.
//

#include <BoundingBox.h>
#include "BVH.h"
#include "Ray.h"

namespace STATS {
    //STAT_MEMORY_COUNTER("Memory/BVH tree", treeBytes);
    //STAT_RATIO("BVH/Primitives per leaf node", totalPrimitives, totalLeafNodes);

    static int totalPrimitives = 0;
    static int totalLeafNodes = 0;
    static int interiorNodes = 0;
    static int leafNodes = 0;
}
struct BVHPrimitiveInfo {
    BVHPrimitiveInfo() : primitiveNumber(0), bounds(),
                         centroid(){ }
    BVHPrimitiveInfo(size_t primitiveNumber, const AxisAlignedBoundingBox &bounds)
            : primitiveNumber(primitiveNumber), bounds(bounds),
              centroid(.5f * bounds.min + .5f * bounds.max) { }
    size_t primitiveNumber;
    AxisAlignedBoundingBox bounds;
    Vector3d centroid;
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
                     int end, int dim){
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
                int end, int dim, AxisAlignedBoundingBox& centroidBounds){
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

int BVHAccel::SplitSAH(std::vector<BVHPrimitiveInfo> &primitiveInfo, int start,
                int end, int dim, AxisAlignedBoundingBox& centroidBounds, AxisAlignedBoundingBox& bounds){
    //<<Partition primitives using approximate SAH>>
    int nPrimitives = end - start;
    int mid = -1;
    if (nPrimitives <= 2) {
        //<<Partition primitives into equally sized subsets>>
        mid = SplitEqualCounts(primitiveInfo, start, end, dim);
    }
    else if (nPrimitives > maxPrimsInNode){ // else return -1 to just create a leaf node
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
            for (int j = i+1; j < nBuckets; ++j) {
                b1 = AxisAlignedBoundingBox::Union(b1, buckets[j].bounds);
                count1 += buckets[j].count;
            }
            cost[i] = 1.f + (count0 * b0.SurfaceArea() +
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
                                                    &primitiveInfo[end-1]+1,
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
                   int maxPrimsInNode, SplitMethod splitMethod)
        : maxPrimsInNode(std::min(255, maxPrimsInNode)),
          splitMethod(splitMethod),
          primitives(std::move(p)) {

    if (primitives.empty())
        return;
    //<<Build BVH from primitives>>
    //<<Initialize primitiveInfo array for primitives>>

    std::vector<BVHPrimitiveInfo> primitiveInfo(primitives.size());
    for (size_t i = 0; i < primitives.size(); ++i)
        primitiveInfo[i] = { i, primitives[i]->sh.bb };

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

    printf("BVH created with %d nodes for %d "
                 "primitives (%.2f MB), arena allocated %.2f MB",
                 totalNodes, (int)primitives.size(),
                 float(totalNodes * sizeof(LinearBVHNode)) /
                 (1024.f * 1024.f),
                 float(totalNodes * sizeof(LinearBVHNode)) /
                 (1024.f * 1024.f));
    //<<Compute representation of depth-first traversal of BVH tree>>
    nodes = new LinearBVHNode[totalNodes]; //AllocAligned<LinearBVHNode>(totalNodes);
    int offset = 0;
    flattenBVHTree(root, &offset);
    assert(totalNodes == offset);

    printf("--- BVH STATS ---\n");
    printf(" Total Primitives: %d\n", STATS::totalPrimitives);
    printf(" Total Leaf Nodes: %d\n", STATS::totalLeafNodes);
    printf(" Interior Nodes:   %d\n", STATS::interiorNodes);
    printf(" Leaf Nodes:       %d\n", STATS::leafNodes);

}

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
        if (centroidBounds.max[dim] == centroidBounds.min[dim] ) {
            //<<Create leaf BVHBuildNode>>
            int firstPrimOffset = orderedPrims.size();
            for (int i = start; i < end; ++i) {
                int primNum = primitiveInfo[i].primitiveNumber;
                orderedPrims.push_back(primitives[primNum]);
            }
            node->InitLeaf(firstPrimOffset, nPrimitives, bounds);
            return node;
        }
        else {
            //<<Partition primitives based on splitMethod>>
            switch (splitMethod) {
                case SplitMethod::Middle: {
                    mid = SplitMiddle(primitiveInfo, start, end, dim, centroidBounds);
                    break;
                }
                case SplitMethod::EqualCounts: {
                    //<<Partition primitives into equally sized subsets>>
                    mid = SplitEqualCounts(primitiveInfo, start, end, dim);
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

bool BVHAccel::Intersect(Ray &ray) const {
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
        if (node->bounds.IntersectBox(ray, invDir, dirIsNeg)) {
            if (node->nPrimitives > 0) {
                // Intersect ray with primitives in leaf BVH node
                for (int i = 0; i < node->nPrimitives; ++i) {
                    // Do not check last collided facet to prevent self intersections
                    if (primitives[node->primitivesOffset + i]->globalId == ray.lastIntersected)
                        continue;
                    else if (primitives[node->primitivesOffset + i]->Intersect(ray)) {
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