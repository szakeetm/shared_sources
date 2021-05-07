/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#include "IntersectAABB_shared.h"
#include "Random.h"
#include "Polygon.h" //IsInPoly
#include "Helper/MathTools.h"
#include <algorithm> //std::min
#include "Simulation/Simulation.h"
#include <tuple>
#include "AABB.h"

bool IntersectBB_new(const AABBNODE& node,const Vector3d& rayPos,const bool& nullRx,const bool& nullRy,const bool& nullRz,const Vector3d& inverseRayDir) {
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
		*dist = Min(t0, t1);
		return (*dist >= 0.0);
	}

	return false;

}


/*std::tuple<bool,SubprocessFacet*,double>*/ void
IntersectTree(MFSim::Particle &currentParticle, const AABBNODE &node, const Vector3d &rayPos,
              const Vector3d &rayDirOpposite, SubprocessFacet *const lastHitBefore, const bool &nullRx,
              const bool &nullRy, const bool &nullRz, const Vector3d &inverseRayDir, bool &found,
              SubprocessFacet *&collidedFacet, double &minLength) {

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
									double time = currentParticle.particleTime + d / 100.0 / currentParticle.velocity;
									double currentOpacity = currentParticle.model->GetOpacityAt(f, time);
									hardHit = ((currentOpacity == 1.0) || (currentParticle.randomGenerator.rnd()<currentOpacity));
#endif

#if defined(SYNRAD)
									hardHit = !((f->sh.opacity < 0.999999 //Partially transparent facet
										&& currentParticle.randomGenerator.rnd()>f->sh.opacity)
										|| (f->sh.reflectType > 10 //Material reflection
										&& currentParticle.model->materials[f->sh.reflectType - 10].hasBackscattering //Has complex scattering
										&& currentParticle.model->materials[f->sh.reflectType - 10].GetReflectionType(currentParticle.energy,
										acos(Dot(currentParticle.direction, f->sh.N)) - PI / 2, currentParticle.randomGenerator.rnd()) == REFL_TRANS));
#endif
									if (hardHit) {

										// Hard hit
										if (d < minLength) {
											minLength = d;
											collidedFacet = f;
											found = true;
                                            currentParticle.tmpFacetVars[collidedFacet->globalId].colU = u;
                                            currentParticle.tmpFacetVars[collidedFacet->globalId].colV = v;
										}
									}
									else {
                                        currentParticle.tmpFacetVars[f->globalId].colDistTranspPass = d;
                                        currentParticle.tmpFacetVars[f->globalId].colU = u;
                                        currentParticle.tmpFacetVars[f->globalId].colV = v;
                                        currentParticle.transparentHitBuffer.push_back(f);
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
                    currentParticle, *(node.left), rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz,
                    inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);
		}
		if (IntersectBB_new(*(node.right), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
            IntersectTree(
                    currentParticle, *(node.right), rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz,
                    inverseRayDir, /*transparentHitFacetPointers,*/ found, collidedFacet, minLength);
		}
	}
}

bool IsInFacet(const SubprocessFacet &f, const double &u, const double &v) {

	/*

	// 2D polygon "is inside" solving
	// Using the "Jordan curve theorem" (we intersect in v direction here)

	int n_updown, n_found, j;
	double x1, x2, y1, y2, a, minx, maxx;

	n_updown = 0;
	n_found = 0;

	for (j = 0; j < f.indices.size() - 1; j++) {

		x1 = f.vertices2[j].u;
		y1 = f.vertices2[j].v;
		x2 = f.vertices2[j + 1].u;
		y2 = f.vertices2[j + 1].v;

		minx = std::min(x1, x2);
		maxx = std::max(x1, x2);

		if (u > minx && u <= maxx) {
			a = (y2 - y1) / (x2 - x1);
			if ((a*(u - x1) + y1) < v) {
				n_updown++;
			}
			else {
				n_updown--;
			}
			n_found++;
		}

	}

	// Last point
	x1 = f.vertices2[j].u;
	y1 = f.vertices2[j].v;
	x2 = f.vertices2[0].u;
	y2 = f.vertices2[0].v;

	minx = std::min(x1, x2);
	maxx = std::max(x1, x2);

	if (u > minx && u <= maxx) {
		a = (y2 - y1) / (x2 - x1);
		if ((a*(u - x1) + y1) < v) {
			n_updown++;
		}
		else {
			n_updown--;
		}
		n_found++;
	}

	if (n_updown<0) n_updown = -n_updown;
	return (((n_found / 2) & 1) ^ ((n_updown / 2) & 1));
	*/

	return IsInPoly(u, v, f.vertices2);

}

std::tuple<bool, SubprocessFacet *, double>
Intersect(MFSim::Particle &currentParticle, const Vector3d &rayPos, const Vector3d &rayDir, const AABBNODE *bvh) {
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
	SubprocessFacet *collidedFacet = nullptr;
	currentParticle.transparentHitBuffer.clear();
	double minLength = 1e100;

    IntersectTree(currentParticle, *bvh, rayPos, -1.0 * rayDir,
                  currentParticle.lastHitFacet,
                  nullRx, nullRy, nullRz, inverseRayDir,
            /*transparentHitFacetPointers,*/ found, collidedFacet, minLength); //output params

	if (found) {

        currentParticle.tmpFacetVars[collidedFacet->globalId].isHit = true;

		// Second pass for transparent hits
		/*for (const auto& tpFacet : currentParticle.transparentHitBuffer){
			if (tpFacet->colDist < minLength) {
                model->RegisterTransparentPass(tpFacet, currentParticle);
			}
		}*/
        // Second pass for transparent hits
        for (auto& tpFacet : currentParticle.transparentHitBuffer){
            if (currentParticle.tmpFacetVars[tpFacet->globalId].colDistTranspPass >= minLength) {
                tpFacet = nullptr;
            }
        }
	}
	return { found, collidedFacet, minLength };

}

/*bool Visible(Simulation *sHandle, Vector3d *c1, Vector3d *c2, SubprocessFacet *f1, SubprocessFacet *f2,
             CurrentParticleStatus &currentParticle) {
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
	currentParticle.transparentHitBuffer.clear();

    IntersectTree(model, *sHandle->model.structures[0].aabbTree, rayPos, -1.0 * rayDir,
                  f1, nullRx, nullRy, nullRz, inverseRayDir, *//*transparentHitFacetPointers,*//* found, collidedFacet,
                  minLength, currentParticle);

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