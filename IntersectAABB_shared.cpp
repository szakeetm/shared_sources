#include <math.h>
#include <stdlib.h>
//#include <malloc.h>
#include <string.h>
#include <math.h>
#include "IntersectAABB_shared.h"
#include "Random.h"
#include "GLApp/MathTools.h"
#include <algorithm> //std::min
#include "Simulation.h"

// AABB tree stuff

// Minimum number of facet inside a BB
#define MINBB    1
#define MAXDEPTH 50

std::tuple<size_t,size_t,size_t> FindBestCuttingPlane(struct AABBNODE *node) {

	// AABB tree balancing

	double centerX = (node->bb.min.x + node->bb.max.x) / 2.0;
	double centerY = (node->bb.min.y + node->bb.max.y) / 2.0;
	double centerZ = (node->bb.min.z + node->bb.max.z) / 2.0;
	size_t rightFromCenterX = 0;
	size_t rightFromCenterY = 0;
	size_t rightFromCenterZ = 0;
	size_t planeType; //1: YZ, 2: XZ, 3: XY
	double best = 1e100;
	size_t nbLeft, nbRight;

	for (size_t i = 0;i<node->nbFacet;i++) {
		FACET *f = node->list[i];
		if (f->sh.center.x > centerX) rightFromCenterZ++;
		if (f->sh.center.y > centerY) rightFromCenterY++;
		if (f->sh.center.z > centerZ) rightFromCenterX++;
	}

	double deviationFromHalfHalf_X = fabs((double)rightFromCenterZ - (double)(node->nbFacet) / 2.0);
	if (deviationFromHalfHalf_X < best) {
		best = deviationFromHalfHalf_X;
		nbLeft = node->nbFacet - rightFromCenterZ;
		nbRight = rightFromCenterZ;
		planeType = 1;
	}
	double deviationFromHalfHalf_Y = fabs((double)rightFromCenterY - (double)(node->nbFacet) / 2.0);
	if (deviationFromHalfHalf_Y < best) {
		best = deviationFromHalfHalf_Y;
		nbLeft = node->nbFacet - rightFromCenterY;
		nbRight = rightFromCenterY;
		planeType = 2;
	}
	double deviationFromHalfHalf_Z = fabs((double)rightFromCenterX - (double)(node->nbFacet) / 2.0);
	if (deviationFromHalfHalf_Z < best) {
		best = deviationFromHalfHalf_Z;
		nbLeft = node->nbFacet - rightFromCenterX;
		nbRight = rightFromCenterX;
		planeType = 3;
	}

	return std::make_tuple(planeType,nbLeft,nbRight);

}

void ComputeBB(struct AABBNODE *node) {

	int i;

	node->bb.max=Vector3d(-1e100,-1e100,-1e100);
	node->bb.min=Vector3d(1e100,1e100,1e100);

	for (i = 0;i<node->nbFacet;i++) {
		FACET *f = node->list[i];
		node->bb.min.x = std::min(f->sh.bb.min.x,node->bb.min.x);
		node->bb.min.y = std::min(f->sh.bb.min.y, node->bb.min.y);
		node->bb.min.z = std::min(f->sh.bb.min.z, node->bb.min.z);
		node->bb.max.x = std::max(f->sh.bb.max.x, node->bb.max.x);
		node->bb.max.y = std::max(f->sh.bb.max.y, node->bb.max.y);
		node->bb.max.z = std::max(f->sh.bb.max.z, node->bb.max.z);
	}

}

struct AABBNODE *BuildAABBTree(FACET **list, const size_t nbFacet, const size_t depth,size_t& maxDepth) {

	size_t    nbl = 0, nbr = 0;
	double m;
	
	maxDepth = std::max(depth, maxDepth); //debug
	if (depth >= MAXDEPTH) return NULL;

	struct AABBNODE *newNode = new AABBNODE();
	newNode->nbFacet = nbFacet;
	newNode->list = list;
	ComputeBB(newNode);
	
	size_t planeType, nbLeft, nbRight;
	std::tie(planeType, nbLeft, nbRight) = FindBestCuttingPlane(newNode);

	if (nbLeft >= MINBB && nbRight >= MINBB) {

		// We can cut
		FACET **lList = new FACET*[nbLeft];
		FACET **rList = new FACET*[nbRight];
		switch (planeType) {

		case 1: // yz
			m = (newNode->bb.min.x + newNode->bb.max.x) / 2.0;
			for (size_t i = 0;i<newNode->nbFacet;i++) {
				FACET *f = newNode->list[i];
				if (f->sh.center.x > m) rList[nbr++] = f;
				else                   lList[nbl++] = f;
			}
			break;

		case 2: // xz
			m = (newNode->bb.min.y + newNode->bb.max.y) / 2.0;
			for (size_t i = 0;i<newNode->nbFacet;i++) {
				FACET *f = newNode->list[i];
				if (f->sh.center.y > m) rList[nbr++] = f;
				else                   lList[nbl++] = f;
			}
			break;

		case 3: // xy
			m = (newNode->bb.min.z + newNode->bb.max.z) / 2.0;
			for (size_t i = 0;i<newNode->nbFacet;i++) {
				FACET *f = newNode->list[i];
				if (f->sh.center.z > m) rList[nbr++] = f;
				else                   lList[nbl++] = f;
			}
			break;

		}
		newNode->left = BuildAABBTree(lList, nbl, depth + 1, maxDepth);
		newNode->right = BuildAABBTree(rList, nbr, depth + 1, maxDepth);
	}

	return newNode;

}

void DestroyAABB(struct AABBNODE *node) {

	if (node != NULL) {
		DestroyAABB(node->left);
		DestroyAABB(node->right);
		delete [] node->list;
		delete node;
	}
}

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

/*
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
*/

/*std::tuple<bool,FACET*,double>*/ void IntersectTree(struct AABBNODE *node, const Vector3d& rayPos, const Vector3d& rayDirOpposite, FACET* const lastHitBefore,
	const bool& nullRx, const bool& nullRy, const bool& nullRz, const Vector3d& inverseRayDir, size_t& intNbTHits, FACET**& THitCache, bool& found, FACET*& collidedFacet, double& minLength) {

	// Returns three values
	// bool: did collision occur?
	// FACET* : closest collided facet
	// double: minimum distance
	// Operates on global variables intNbTHits and THits[] to avoid passing these on heap

	// Method: 3x3 Sytem solving for ray/rectangle intersection. 
	// Solve the vector equation u*U + v*V + d*D = Z (using Cramer's rule)
	// nuv = u^v (for faster calculation)

	/*bool found = false;
	FACET* collidedFacet = lastHitBefore;
	double minLength=minLengthSoFar;*/

	if (node->left == NULL || node->right == NULL) { // Leaf

		for (size_t i = 0;i<node->nbFacet;i++) {

			FACET* f = node->list[i];

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
#ifdef MOLFLOW
									double time = sHandle->flightTimeCurrentParticle + d / 100.0 / sHandle->velocityCurrentParticle;
									double currentOpacity = GetOpacityAt(f, time);
									hardHit = ((currentOpacity == 1.0) || (rnd()<currentOpacity));
#endif

#ifdef SYNRAD
									hardHit = !((f->sh.opacity < 0.999999 //Partially transparent facet
										&& rnd()>f->sh.opacity)
										|| (f->sh.reflectType > 10 //Material reflection
										&& sHandle->materials[f->sh.reflectType - 10].hasBackscattering //Has complex scattering
										&& sHandle->materials[f->sh.reflectType - 10].GetReflectionType(sHandle->energy,
										acos(Dot(sHandle->pDir, f->sh.N)) - PI / 2, rnd()) == REFL_TRANS));
#endif
									if (hardHit) {

										// Hard hit
										if (d < minLength) {
											minLength = d;
											collidedFacet = f;
											found = true;
											f->colU = u;
											f->colV = v;
										}
									}
									else {
										// Pass on partial transparent facet
										//if (f->sh.isProfile || f->hits) {
											f->colDist = d;
											f->colU = u;
											f->colV = v;
											if (intNbTHits<MAX_THIT)
												THitCache[intNbTHits++] = f;
										//}
									}
								} // IsInFacet
							} // d range
						} // u range
					} // v range
				} // det==0
			} // dot<0
		} // end for

	} /* end Leaf */ else {
		if (IntersectBB_new(*(node->left), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
			IntersectTree(node->left, rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz, inverseRayDir, intNbTHits, THitCache, found, collidedFacet, minLength);
		}
		if (IntersectBB_new(*(node->right), rayPos, nullRx, nullRy, nullRz, inverseRayDir)) {
			IntersectTree(node->right, rayPos, rayDirOpposite, lastHitBefore, nullRx, nullRy, nullRz, inverseRayDir, intNbTHits, THitCache, found, collidedFacet, minLength);
		}
		double a = 5;
	}
}

bool IsInFacet(const FACET &f, const double &u, const double &v) {

	// 2D polygon "is inside" solving
	// Using the "Jordan curve theorem" (we intersect in v direction here)

	int n_updown, n_found, j;
	double x1, x2, y1, y2, a, minx, maxx;

	n_updown = 0;
	n_found = 0;

	for (j = 0; j < f.sh.nbIndex - 1; j++) {

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

}
