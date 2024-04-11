#pragma once

#include "BoundingBox.h"
#include "Vector.h" //AABB

// AABBTree node
class Simulation_Abstract;
class SimulationFacet;
class Ray;
namespace MFSim {
    class ParticleTracer;
}
		 
bool IsInFacet(const SimulationFacet &f, const double u, const double v);
bool IntersectBox(const AxisAlignedBoundingBox& targetBox, const Ray& ray, const Vector3d& invDir, const int dirIsNeg[3]);

