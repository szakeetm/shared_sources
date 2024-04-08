
#pragma once

// Temporary transparent hit
//#include "Simulation/SynradSimGeom.h"
//#include "../src/Simulation/Particle.h" //synrad or molflow
#include "BoundingBox.h"
#include "Vector.h" //AABB

//#include "../src/Simulation.h" //SubprocessFacet

// AABBTree node
class Simulation_Abstract;
class SimulationFacet;
class Ray;
namespace MFSim {
    class ParticleTracer;
}

class AABBNODE {
public:
	AABBNODE();
	~AABBNODE();
	void ComputeBB();
	std::tuple<size_t, size_t, size_t> FindBestCuttingPlane();
	AxisAlignedBoundingBox bb;
	AABBNODE *left;
	AABBNODE *right;
	std::vector<SimulationFacet*> facets;
	size_t GetMemSize();
};

AABBNODE *BuildAABBTree(const std::vector<SimulationFacet *> &facets, const size_t depth, size_t& maxDepth);

void IntersectTree(std::shared_ptr<MFSim::ParticleTracer> currentParticleTracer, const AABBNODE &node, const Vector3d &rayPos,
                   const Vector3d &rayDirOpposite, SimulationFacet *const lastHitBefore, const bool nullRx,
                   const bool nullRy, const bool nullRz, const Vector3d &inverseRayDir, bool &found,
                   SimulationFacet *&collidedFacet, double &minLength);
std::tuple<bool, SimulationFacet *, double>
Intersect(std::shared_ptr<MFSim::ParticleTracer> currentParticleTracer, const Vector3d &rayPos, const Vector3d &rayDir, const AABBNODE *bvh);
/*bool Visible(Simulation *sHandle, Vector3d *c1, Vector3d *c2, SubprocessFacet *f1, SubprocessFacet *f2,
             CurrentParticleStatus &currentParticleTracer);*/
bool IsInFacet(const SimulationFacet &f, const double u, const double v);
bool IntersectBox(const AxisAlignedBoundingBox& targetBox, const Ray& ray, const Vector3d& invDir, const int dirIsNeg[3]);

//Vector3d PolarToCartesian(const SubprocessFacet *const collidedFacet, const double theta, const double phi, const bool reverse); //sets sHandle->currentParticleTracer.direction
//std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN);

