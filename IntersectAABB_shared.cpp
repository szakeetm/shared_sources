
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
#include "GLApp/GLTypes.h"

// AABB tree stuff

// Minimum number of facet inside a BB
#define MINBB    1
#define MAXDEPTH 50

bool IsInFacet(const SimulationFacet &f, const double u, const double v) {

	return IsInPoly(u, v, f.vertices2);

}


//! Ray-AABB intersection, given in the inverse direction of the ray and a handy array dirIsNeg that gives a factor for negative directions (dir < 0)
//Performance critical! 35% of ray-tracing CPU usage
bool IntersectBox(const AxisAlignedBoundingBox& bounds, const Ray& ray, const Vector3d& invDir, const int dirIsNeg[3]) {
	
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