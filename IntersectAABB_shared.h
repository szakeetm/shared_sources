#pragma once

// Temporary transparent hit
#include "Vector.h" //AABB
#include "Simulation.h" //FACET

// AABBTree node

struct AABBNODE {

	AABB             bb;
	struct AABBNODE *left;
	struct AABBNODE *right;
	FACET          **list;
	size_t           nbFacet;

};

struct AABBNODE *BuildAABBTree(FACET **list,const size_t nbFacet,const size_t depth);
std::tuple<size_t, size_t, size_t> FindBestCuttingPlane(struct AABBNODE *node);
void ComputeBB(struct AABBNODE *node);
void DestroyAABB(struct AABBNODE *node);
std::tuple<bool, FACET*, double> IntersectTree(struct AABBNODE *node, const Vector3d& rayPos, const Vector3d& rayDir, const double& minLengthSoFar, FACET* const lastHitBefore,
	const bool& nullRx, const bool& nullRy, const bool& nullRz, const Vector3d& inverseRayDir, size_t* intNbTHits, FACET** THitCache);
std::tuple<bool, FACET*, double> Intersect(const Vector3d& rayPos, const Vector3d& rayDir, FACET* lastHitFacet, FACET** THitCache);
bool Visible(Vector3d *c1,Vector3d *c2,FACET *f1,FACET *f2,FACET** THitCache);
bool IsInFacet(const FACET &f,const double &u,const double &v);