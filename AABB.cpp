//
// Created by pbahr on 07/04/2021.
//

#include "AABB.h"
#include "SimulationFacet.h"
// AABB tree stuff

// Minimum number of facet inside a BB
#define MINBB    1
#define MAXDEPTH 50

std::tuple<size_t,size_t,size_t> AABBNODE::FindBestCuttingPlane() {

    // AABB tree balancing

    double centerX = (bb.min.x + bb.max.x) / 2.0;
    double centerY = (bb.min.y + bb.max.y) / 2.0;
    double centerZ = (bb.min.z + bb.max.z) / 2.0;
    size_t rightFromCenterX = 0;
    size_t rightFromCenterY = 0;
    size_t rightFromCenterZ = 0;
    size_t planeType; //1: YZ, 2: XZ, 3: XY
    double best = 1e100;
    size_t nbLeft, nbRight;

    for (const auto& f : facets) {
        if (f->sh.center.x > centerX) rightFromCenterZ++;
        if (f->sh.center.y > centerY) rightFromCenterY++;
        if (f->sh.center.z > centerZ) rightFromCenterX++;
    }

    double deviationFromHalfHalf_X = fabs((double)rightFromCenterZ - (double)(facets.size()) / 2.0);
    if (deviationFromHalfHalf_X < best) {
        best = deviationFromHalfHalf_X;
        nbLeft = facets.size() - rightFromCenterZ;
        nbRight = rightFromCenterZ;
        planeType = 1;
    }
    double deviationFromHalfHalf_Y = fabs((double)rightFromCenterY - (double)(facets.size()) / 2.0);
    if (deviationFromHalfHalf_Y < best) {
        best = deviationFromHalfHalf_Y;
        nbLeft = facets.size() - rightFromCenterY;
        nbRight = rightFromCenterY;
        planeType = 2;
    }
    double deviationFromHalfHalf_Z = fabs((double)rightFromCenterX - (double)(facets.size()) / 2.0);
    if (deviationFromHalfHalf_Z < best) {
        best = deviationFromHalfHalf_Z;
        nbLeft = facets.size() - rightFromCenterX;
        nbRight = rightFromCenterX;
        planeType = 3;
    }

    return { planeType,nbLeft,nbRight };

}

void AABBNODE::ComputeBB() {

    bb.max=Vector3d(-1e100,-1e100,-1e100);
    bb.min=Vector3d(1e100,1e100,1e100);

    for (const auto& f : facets) {
        bb.min.x = std::min(f->sh.bb.min.x,bb.min.x);
        bb.min.y = std::min(f->sh.bb.min.y, bb.min.y);
        bb.min.z = std::min(f->sh.bb.min.z, bb.min.z);
        bb.max.x = std::max(f->sh.bb.max.x, bb.max.x);
        bb.max.y = std::max(f->sh.bb.max.y, bb.max.y);
        bb.max.z = std::max(f->sh.bb.max.z, bb.max.z);
    }

}

AABBNODE *BuildAABBTree(const std::vector<SimulationFacet*>& facets, const size_t depth,size_t& maxDepth) {

    size_t    nbl = 0, nbr = 0;
    double m;

    maxDepth = std::max(depth, maxDepth); //debug
    if (depth >= MAXDEPTH) return nullptr;

    AABBNODE *newNode = new AABBNODE();
    newNode->facets = facets;
    newNode->ComputeBB();

    auto [planeType, nbLeft, nbRight] = newNode->FindBestCuttingPlane();

    if (nbLeft >= MINBB && nbRight >= MINBB) {

        // We can cut
        std::vector<SimulationFacet*> lList(nbLeft);
        std::vector<SimulationFacet*> rList(nbRight);
        switch (planeType) {
            case 1: // yz
                m = (newNode->bb.min.x + newNode->bb.max.x) / 2.0;
                for (const auto& f : newNode->facets) {
                    if (f->sh.center.x > m) rList[nbr++] = f;
                    else                   lList[nbl++] = f;
                }
                break;

            case 2: // xz
                m = (newNode->bb.min.y + newNode->bb.max.y) / 2.0;
                for (const auto& f : newNode->facets) {
                    if (f->sh.center.y > m) rList[nbr++] = f;
                    else                   lList[nbl++] = f;
                }
                break;

            case 3: // xy
                m = (newNode->bb.min.z + newNode->bb.max.z) / 2.0;
                for (const auto& f : newNode->facets) {
                    if (f->sh.center.z > m) rList[nbr++] = f;
                    else                   lList[nbl++] = f;
                }
                break;
            default:
                delete newNode;
                std::cerr << "Unknown planeType: "<< planeType << "(/3)" << std::endl;
                return nullptr;
        }
        newNode->left = BuildAABBTree(lList, depth + 1, maxDepth);
        newNode->right = BuildAABBTree(rList, depth + 1, maxDepth);
    }

    return newNode;

}