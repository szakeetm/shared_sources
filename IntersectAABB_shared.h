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
#pragma once

// Temporary transparent hit
//#include "Simulation/SynradSimGeom.h"
#include "../src/Simulation/Particle.h" //synrad or molflow
#include "Vector.h" //AABB
//#include "../src/Simulation.h" //SubprocessFacet

// AABBTree node
class Simulation_Abstract;
class SimulationFacet;

class AABBNODE {
public:
	AABBNODE();
	~AABBNODE();
	void ComputeBB();
	std::tuple<size_t, size_t, size_t> FindBestCuttingPlane();
	AxisAlignedBoundingBox             bb;
	AABBNODE *left;
	AABBNODE *right;
	std::vector<SimulationFacet*> facets;
};

AABBNODE *BuildAABBTree(const std::vector<SimulationFacet *> &facets, const size_t depth, size_t& maxDepth);

void IntersectTree(MFSim::ParticleTracer &currentParticleTracer, const AABBNODE &node, const Vector3d &rayPos,
                   const Vector3d &rayDirOpposite, SimulationFacet *const lastHitBefore, const bool nullRx,
                   const bool nullRy, const bool nullRz, const Vector3d &inverseRayDir, bool &found,
                   SimulationFacet *&collidedFacet, double &minLength);
std::tuple<bool, SimulationFacet *, double>
Intersect(MFSim::ParticleTracer &currentParticleTracer, const Vector3d &rayPos, const Vector3d &rayDir, const AABBNODE *bvh);
/*bool Visible(Simulation *sHandle, Vector3d *c1, Vector3d *c2, SubprocessFacet *f1, SubprocessFacet *f2,
             CurrentParticleStatus &currentParticleTracer);*/
bool IsInFacet(const SimulationFacet &f, const double u, const double v);
bool IntersectBox(const AxisAlignedBoundingBox& targetBox, const Ray& ray, const Vector3d& invDir, const int dirIsNeg[3]);

//Vector3d PolarToCartesian(const SubprocessFacet *const collidedFacet, const double theta, const double phi, const bool reverse); //sets sHandle->currentParticleTracer.direction
//std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN);