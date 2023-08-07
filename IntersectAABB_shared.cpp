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
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "SimulationFacet.h"
#include "IntersectAABB_shared.h"
#include "Random.h"
#include "Polygon.h" //IsInPoly
#include "Helper/MathTools.h"
#include <algorithm> //std::min
#include "Simulation/MolflowSimulation.h"
#include <tuple>
#include "RayTracing/Ray.h"
#include "BoundingBox.h"
#include "Simulation/Particle.h"

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

size_t AABBNODE::GetMemSize() {
	size_t sum = 0;
	if (left) {
		sum += left->GetMemSize();
	}
	else sum += sizeof(left);
	if (right) {
		sum += right->GetMemSize();
	}
	else sum += sizeof(right);
	return sum;
}

AABBNODE *BuildAABBTree(const std::vector<SimulationFacet *> &facets, const size_t depth, size_t& maxDepth) {

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

bool IntersectBB_new(const AABBNODE& node,const Vector3d& rayPos,const bool nullRx,const bool nullRy,const bool nullRz,const Vector3d& inverseRayDir) {
	double tNear, tFar;
	//X component

	if (nullRx) {
		if (rayPos.x < node.bb.min.x || rayPos.x > node.bb.max.x) {
			return false;
		} 
		else {
			tNear = -1e100;
			tFar = 1e100;
		}
	}
	else {
		double intersection1 = (node.bb.min.x - rayPos.x) * inverseRayDir.x;
		double intersection2 = (node.bb.max.x - rayPos.x) * inverseRayDir.x;
		tNear = std::min(intersection1, intersection2);
		tFar = std::max(intersection1, intersection2);
		if (tFar < 0.0) return false;
	}

	//Y component
	if (nullRy) {
		if (rayPos.y < node.bb.min.y || rayPos.y > node.bb.max.y) return false;
	}
	else {
		double intersection1 = (node.bb.min.y - rayPos.y) * inverseRayDir.y;
		double intersection2 = (node.bb.max.y - rayPos.y) * inverseRayDir.y;
		tNear = std::max(tNear, std::min(intersection1, intersection2));
		tFar = std::min(tFar, std::max(intersection1, intersection2));
		if (tNear>tFar || tFar<0.0) return false;
	}

	//Z component
	if (nullRz) {
			if (rayPos.z < node.bb.min.z || rayPos.z > node.bb.max.z) return false;   
	}
	else {
		double intersection1 = (node.bb.min.z - rayPos.z) * inverseRayDir.z;
		double intersection2 = (node.bb.max.z - rayPos.z) * inverseRayDir.z;
		tNear = std::max(tNear, std::min(intersection1, intersection2));
		tFar = std::min(tFar, std::max(intersection1, intersection2));
		if (tNear>tFar || tFar<0.0) return false;
	}

	return true;
}

/*
// Ray AABB intersection check (slabs method)
#define IntersectBB(n,lab)                                                   \
	\
	AABBHit = false;                                                           \
	\
	if( nullRx ) {                                                             \
	if( rayPos->x < (n)->bb.min.x || rayPos->x > (n)->bb.max.x ) goto lab;   \
	tNear =  -1e100;                                                         \
	tFar  =   1e100;                                                         \
	} else {                                                                   \
	it1 = ((n)->bb.min.x - rayPos->x)*iRx;                                   \
	it2 = ((n)->bb.max.x - rayPos->x)*iRx;                                   \
	tNear = std::min(it1,it2);												\
	tFar  = std::max(it1,it2);                                              \
	}                                                                          \
	if( tFar<0.0 ) goto lab;                                                   \
	\
	if( nullRy ) {                                                             \
	if( rayPos->y < (n)->bb.min.y || rayPos->y > (n)->bb.max.y ) goto lab;   \
	} else {                                                                   \
	it1 = ((n)->bb.min.y - rayPos->y)*iRy;                                   \
	it2 = ((n)->bb.max.y - rayPos->y)*iRy;                                   \
	tNear = std::max(tNear, std::min(it1,it2));								\
	tFar  = std::min(tFar , std::max(it1,it2));								\                                                                      
	}                                                                          \
	if( tNear>tFar || tFar<0.0 ) goto lab;                                     \
	if( nullRz ) {                                                             \
	if( rayPos->z < (n)->bb.min.z || rayPos->z > (n)->bb.max.z ) goto lab;   \
	}
	else { \
			it1 = ((n)->bb.min.y - rayPos->y)*iRy;                                   \
			it2 = ((n)->bb.max.y - rayPos->y)*iRy;                                   \
			tNear = std::max(tNear, std::min(it1, it2));								\
			tFar = std::min(tFar, std::max(it1, it2));								\
	}                                                                         \
	if( tNear>tFar || tFar<0.0 ) goto lab;                                     \
	AABBHit = true;                                                            \
lab:

*/


//Unused as of 2017/09/25
bool RaySphereIntersect(Vector3d *center, double radius, Vector3d *rPos, Vector3d *rDir, double *dist) {

	// Perform ray-sphere intersection
	double B, C, D;
	Vector3d s;
	s.x = (rPos->x - center->x);
	s.y = (rPos->y - center->y);
	s.z = (rPos->z - center->z);

	//|rDir|=1 => A=1
	//A = DOT3(rDir->x,rDir->y,rDir->z,rDir->x,rDir->y,rDir->z);
	B = 2.0 * Dot(*rDir, s);
	C = Dot(s, s) - radius*radius;
	D = B*B - 4 * C;

	if (D >= 0.0) {
		double rD = sqrt(D);
		double t0 = (-B - rD) / 2.0;
		double t1 = (-B + rD) / 2.0;
		*dist = std::min(t0, t1);
		return (*dist >= 0.0);
	}

	return false;

}


/*std::tuple<bool,SubprocessFacet*,double>*/ void
IntersectTree(std::shared_ptr<MFSim::ParticleTracer> currentParticleTracer, const AABBNODE &node, const Vector3d &rayPos,
              const Vector3d &rayDirOpposite, SimulationFacet *const lastHitBefore, const bool nullRx,
              const bool nullRy, const bool nullRz, const Vector3d &inverseRayDir, bool &found,
              SimulationFacet *&collidedFacet, double &minLength) {

	// Returns three values
	// bool: did collision occur?
	// SubprocessFacet* : closest collided facet
	// double: minimum distance

	// Method: 3x3 Sytem solving for ray/rectangle intersection. 
	// Solve the vectosr equation u*U + v*V + d*D = Z (using Cramer's rule)
	// nuv = u^v (for faster calculation)

	/*bool found = false;
	SubprocessFacet* collidedFacet = lastHitBefore;
	double minLength=minLengthSoFar;*/

	if (node.left == nullptr || node.right == nullptr) { // Leaf

		for (const auto &f : node.facets) {
			// Do not check last collided facet
			if (f == lastHitBefore)
				continue;

			double det = Dot(f->sh.Nuv, rayDirOpposite);
			// Eliminate "back facet"
			if ((f->sh.is2sided) || (det > 0.0)) { //If 2-sided or if ray going opposite facet normal

				double u, v, d;
				// Ray/rectangle instersection. Find (u,v,dist) and check 0<=u<=1, 0<=v<=1, dist>=0
				
				if (det != 0.0) {

					double iDet = 1.0 / det;
					Vector3d intZ = rayPos - f->sh.O;

					u = iDet * DET33(intZ.x, f->sh.V.x, rayDirOpposite.x,
						intZ.y, f->sh.V.y, rayDirOpposite.y,
						intZ.z, f->sh.V.z, rayDirOpposite.z);

					if (u >= 0.0 && u <= 1.0) {

						v = iDet * DET33(f->sh.U.x, intZ.x, rayDirOpposite.x,
							f->sh.U.y, intZ.y, rayDirOpposite.y,
							f->sh.U.z, intZ.z, rayDirOpposite.z);

						if (v >= 0.0 && v <= 1.0) {

							d = iDet * Dot(f->sh.Nuv, intZ);

							if (d>0.0) {

								// Now check intersection with the facet polygon (in the u,v space)
								// This check could be avoided on rectangular facet.
								if (IsInFacet(*f, u, v)) {
									bool hardHit;
#if defined(MOLFLOW)
									double time = currentParticleTracer->ray.time + d / 100.0 / currentParticleTracer->velocity;
									double currentOpacity = currentParticleTracer->model->GetOpacityAt(f, time);
									hardHit = ((currentOpacity == 1.0) || (currentParticleTracer->randomGenerator.rnd()<currentOpacity));
#endif

#if defined(SYNRAD)
                                    hardHit = !((f->sh.opacity < 0.999999 //Partially transparent facet
										&& currentParticleTracer->randomGenerator.rnd()>f->sh.opacity)
										|| (f->sh.reflectType > 10 //Material reflection
										&& currentParticleTracer->model->materials[f->sh.reflectType - 10].hasBackscattering //Has complex scattering
										&& currentParticleTracer->model->materials[f->sh.reflectType - 10].GetReflectionType(currentParticleTracer->energy,
										acos(Dot(-1.0 * rayDirOpposite, f->sh.N)) - PI / 2, currentParticleTracer->randomGenerator.rnd()) == REFL_TRANS));
#endif
									if (hardHit) {

										// Hard hit
										if (d < minLength) {
											minLength = d;
											collidedFacet = f;
											found = true;
                                            currentParticleTracer->tmpFacetVars[collidedFacet->globalId].colU = u;
                                            currentParticleTracer->tmpFacetVars[collidedFacet->globalId].colV = v;
										}
									}
									else {
                                        currentParticleTracer->tmpFacetVars[f->globalId].colDistTranspPass = d;
                                        currentParticleTracer->tmpFacetVars[f->globalId].colU = u;
                                        currentParticleTracer->tmpFacetVars[f->globalId].colV = v;
                                        currentParticleTracer->transparentHitBuffer.push_back(f);
									}
								} // IsInFacet
							} // d range
						} // u range
					} // v range
				} // det==0
			} // dot<0
		} // end for

	} /* end Leaf */ else {

		if (IntersectBB_new(*(node.left), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
            IntersectTree(
                    currentParticleTracer, *(node.left), rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz,
                    inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);
		}
		if (IntersectBB_new(*(node.right), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
            IntersectTree(
                    currentParticleTracer, *(node.right), rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz,
                    inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);
		}
	}
}

bool IsInFacet(const SimulationFacet &f, const double u, const double v) {

	return IsInPoly(u, v, f.vertices2);

}

std::tuple<bool, SimulationFacet *, double>
Intersect(std::shared_ptr<MFSim::ParticleTracer> currentParticleTracer, const Vector3d &rayPos, const Vector3d &rayDir, const AABBNODE *bvh) {
	// Source ray (rayDir vector must be normalized)
	// lastHit is to avoid detecting twice the same collision
	// returns bool found (is there a collision), pointer to collided facet, double d (distance to collision)

	bool nullRx = (rayDir.x == 0.0);
	bool nullRy = (rayDir.y == 0.0);
	bool nullRz = (rayDir.z == 0.0);
	Vector3d inverseRayDir;
	if (!nullRx) inverseRayDir.x = 1.0 / rayDir.x;
	if (!nullRy) inverseRayDir.y = 1.0 / rayDir.y;
	if (!nullRz) inverseRayDir.z = 1.0 / rayDir.z;

	//Global variables, easier for recursion:

	//Output values
	bool found = false;
	SimulationFacet *collidedFacet = nullptr;
	currentParticleTracer->transparentHitBuffer.clear();
	double minLength = 1e100;

    IntersectTree(currentParticleTracer, *bvh, rayPos, -1.0 * rayDir,
                  currentParticleTracer->lastHitFacet,
                  nullRx, nullRy, nullRz, inverseRayDir,
            /*transparentHitFacetPointers,*/ found, collidedFacet, minLength); //output params

	if (found) {

        currentParticleTracer->tmpFacetVars[collidedFacet->globalId].isHit = true;

		// Second pass for transparent hits
		/*for (const auto& tpFacet : currentParticleTracer->transparentHitBuffer){
			if (tpFacet->colDist < minLength) {
                model->RegisterTransparentPass(tpFacet, currentParticleTracer);
			}
		}*/
        // Second pass for transparent hits
        for (auto& tpFacet : currentParticleTracer->transparentHitBuffer){
            if (currentParticleTracer->tmpFacetVars[tpFacet->globalId].colDistTranspPass >= minLength) {
                tpFacet = nullptr;
            }
        }
	}
	return { found, collidedFacet, minLength };

}

//! Ray-AABB intersection, given in the inverse direction of the ray and a handy array dirIsNeg that gives a factor for negative directions (dir < 0)
bool IntersectBox(const AxisAlignedBoundingBox& targetBox, const Ray& ray, const Vector3d& invDir, const int dirIsNeg[3]) {
	const AxisAlignedBoundingBox& bounds = targetBox;
	// Check for ray intersection against $x$ and $y$ slabs
	double tMin = (bounds[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
	double tMax = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x;
	double tyMin = (bounds[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
	double tyMax = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y;

	constexpr double precalc_1_2_gamma3 = 1 + 2 * gamma(3);

	// Update _tMax_ and _tyMax_ to ensure robust bounds intersection
	tMax *= precalc_1_2_gamma3;
	tyMax *= precalc_1_2_gamma3;
	if (tMin > tyMax || tyMin > tMax) return false;
	if (tyMin > tMin) tMin = tyMin;
	if (tyMax < tMax) tMax = tyMax;

	// Check for ray intersection against $z$ slab
	double tzMin = (bounds[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
	double tzMax = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z;

	// Update _tzMax_ to ensure robust bounds intersection
	tzMax *= precalc_1_2_gamma3;
	if (tMin > tzMax || tzMin > tMax) return false;
	if (tzMin > tMin) tMin = tzMin;
	if (tzMax < tMax) tMax = tzMax;
	return (tMin < ray.tMax) && (tMax > 0);
}

/*bool Visible(Simulation *sHandle, Vector3d *c1, Vector3d *c2, SubprocessFacet *f1, SubprocessFacet *f2,
             CurrentParticleStatus &currentParticleTracer) {
	//For AC matrix calculation, used only in MolFlow

	Vector3d rayPos = *c1;
	Vector3d rayDir = *c2 - *c1;

	bool nullRx = (rayDir.x == 0.0);
	bool nullRy = (rayDir.y == 0.0);
	bool nullRz = (rayDir.z == 0.0);
	Vector3d inverseRayDir;
	if (!nullRx) inverseRayDir.x = 1.0 / rayDir.x;
	if (!nullRy) inverseRayDir.y = 1.0 / rayDir.y;
	if (!nullRz) inverseRayDir.z = 1.0 / rayDir.z;

	//Global variables, easier for recursion:
	size_t intNbTHits = 0;

	//Output values
	bool found;
	SubprocessFacet *collidedFacet;
	double minLength;

	//std::vector<SubprocessFacet*> transparentHitFacetPointers;
	currentParticleTracer->transparentHitBuffer.clear();

    IntersectTree(model, *sHandle->model.structures[0].aabbTree, rayPos, -1.0 * rayDir,
                  f1, nullRx, nullRy, nullRz, inverseRayDir, *//*transparentHitFacetPointers,*//* found, collidedFacet,
                  minLength, currentParticleTracer);

	if (found) {
		if (collidedFacet != f2) {
			// Obstacle found
			return false;
		}
	}

	return true;
}*/

AABBNODE::AABBNODE()
{
	left = right = nullptr;
}

AABBNODE::~AABBNODE()
{
	SAFE_DELETE(left);
	SAFE_DELETE(right);
}