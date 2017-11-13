#pragma once

// Temporary transparent hit
#include "Vector.h" //AABB
#include "Simulation.h" //SubprocessFacet

// AABBTree node

struct AABBNODE {

	AABB             bb;
	struct AABBNODE *left;
	struct AABBNODE *right;
	SubprocessFacet          **list;
	size_t           nbFacet;

};

struct AABBNODE *BuildAABBTree(SubprocessFacet **list,const size_t nbFacet,const size_t depth,size_t& maxDepth);
std::tuple<size_t, size_t, size_t> FindBestCuttingPlane(struct AABBNODE *node);
void ComputeBB(struct AABBNODE *node);
void DestroyAABB(struct AABBNODE *node);
void IntersectTree(struct AABBNODE *node, const Vector3d& rayPos, const Vector3d& rayDirOpposite, SubprocessFacet* const lastHitBefore,
	const bool& nullRx, const bool& nullRy, const bool& nullRz, const Vector3d& inverseRayDir,
	size_t& intNbTHits, SubprocessFacet**& THitCache, bool& found, SubprocessFacet*& collidedFacet, double& minLength);
std::tuple<bool, SubprocessFacet*, double> Intersect(const Vector3d& rayPos, const Vector3d& rayDir, SubprocessFacet**& THitCache);
bool Visible(Vector3d *c1,Vector3d *c2,SubprocessFacet *f1,SubprocessFacet *f2,SubprocessFacet** THitCache);
bool IsInFacet(const SubprocessFacet &f,const double &u,const double &v);

void PolarToCartesian(SubprocessFacet* collidedFacet, const double& theta, const double& phi, const bool& reverse); //sets sHandle->pDir
std::tuple<double, double> CartesianToPolar(const Vector3d& normU, const Vector3d& normV, const Vector3d& normN);