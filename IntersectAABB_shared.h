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
#pragma once

// Temporary transparent hit
//#include <Simulation/Particle.h>
#include "Vector.h" //AABB
//#include "../src/Simulation.h" //SubprocessFacet

// AABBTree node
namespace MFSim {
    class Particle;
}

class AABBNODE;
class Simulation;
struct SubprocessFacet;

void IntersectTree(MFSim::Particle &currentParticle, const AABBNODE &node, const Vector3d &rayPos,
                   const Vector3d &rayDirOpposite, SubprocessFacet *const lastHitBefore, const bool &nullRx,
                   const bool &nullRy, const bool &nullRz, const Vector3d &inverseRayDir, bool &found,
                   SubprocessFacet *&collidedFacet, double &minLength);
std::tuple<bool, SubprocessFacet *, double>
Intersect(MFSim::Particle &currentParticle, const Vector3d &rayPos, const Vector3d &rayDir, const AABBNODE *bvh);
/*bool Visible(Simulation *sHandle, Vector3d *c1, Vector3d *c2, SubprocessFacet *f1, SubprocessFacet *f2,
             CurrentParticleStatus &currentParticle);*/
bool IsInFacet(const SubprocessFacet &f,const double &u,const double &v);
//Vector3d PolarToCartesian(const SubprocessFacet *const collidedFacet, const double& theta, const double& phi, const bool& reverse); //sets sHandle->currentParticle.direction
//std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN);