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

// M_PI define
#ifdef _WIN32
#define _USE_MATH_DEFINES // activate defines, e.g. M_PI_2
#endif
#include <cmath>

#include "FacetData.h"
#include "Polygon.h"
#include "Helper/MathTools.h"
#include "RayTracing/RTHelper.h" // SimulationFacetTempVar
#include "RayTracing/Ray.h" // hitlink

#if defined(SYNRAD)
bool MaterialSurface::IsHardHit(const Ray &r) {
    return !((opacity < 0.999999 //Partially transparent facet
              && r.rng->rnd() > opacity)
             || (mat != nullptr &&/*this->sh.reflectType > 10 //Material reflection
                     && */mat->hasBackscattering //Has complex scattering
                 && mat->GetReflectionType(reinterpret_cast<Synpay *>(r.pay)->energy,
                                           acos(Dot(r.direction, N)) - M_PI_2, r.rng->rnd()) == REFL_TRANS));

    /*if(opacity == 1.0)
            return true;
        else if(opacity == 0.0)
            return false;
        else if(r.rng->rnd() < opacity)
            return true;
        else if(mat->hasBackscattering
                && mat->GetReflectionType(reinterpret_cast<Synpay*>(r.pay)->energy,
                                          acos(Dot(r.direction, N)) - M_PI_2, r.rng->rnd()) == REFL_TRANS)
            return true;
        else
            return false;*/
}
#endif

bool Facet::Intersect(Ray &ray) {
    //++iSCount;
    Vector3d rayDirOpposite(-1.0 * ray.direction);
    double det = Dot(this->sh.Nuv, rayDirOpposite);

    // Eliminate "back facet"
    if ((this->sh.is2sided) || (det > 0.0)) { //If 2-sided or if ray going opposite facet normal

        double u, v, d;
        // Ray/rectangle instersection. Find (u,v,dist) and check 0<=u<=1, 0<=v<=1, dist>=0

        if (det != 0.0) {

            double iDet = 1.0 / det;
            Vector3d intZ = ray(0) - this->sh.O;

            u = iDet * DET33(intZ.x, this->sh.V.x, rayDirOpposite.x,
                             intZ.y, this->sh.V.y, rayDirOpposite.y,
                             intZ.z, this->sh.V.z, rayDirOpposite.z);

            if (u >= 0.0 && u <= 1.0) {

                v = iDet * DET33(this->sh.U.x, intZ.x, rayDirOpposite.x,
                                 this->sh.U.y, intZ.y, rayDirOpposite.y,
                                 this->sh.U.z, intZ.z, rayDirOpposite.z);

                if (v >= 0.0 && v <= 1.0) {

                    d = iDet * Dot(this->sh.Nuv, intZ);

                    if (d>0.0) {

                        // Now check intersection with the facet polygon (in the u,v space)
                        // This check could be avoided on rectangular facet.
                        if (IsInPoly(u, v, vertices2)) {
                            bool hardHit = this->surf->IsHardHit(ray);
/*#if defined(SYNRAD)

                            if(typeid(*surf) == typeid(MaterialSurface))
                            hardHit &= (this->sh.reflectType > 10 //Material reflection
                                        && currentParticleTracer.model->materials[this->sh.reflectType - 10].hasBackscattering //Has complex scattering
                                        && currentParticleTracer.model->materials[this->sh.reflectType - 10].GetReflectionType(currentParticleTracer.energy,
                                                                                                                         acos(Dot(currentParticleTracer.direction, this->sh.N)) - PI / 2, currentParticleTracer.randomGenerator.rnd()) == REFL_TRANS));

#endif*/
                            /*#if defined(MOLFLOW)
                            double time = ray.time + d / 100.0 / currentParticleTracer.velocity;
                            double currentOpacity = currentParticleTracer.model->GetOpacityAt(f, time);
                            hardHit = ((currentOpacity == 1.0) || (currentParticleTracer.randomGenerator.rnd()<currentOpacity));
#endif

#if defined(SYNRAD)
                            hardHit = !((this->sh.opacity < 0.999999 //Partially transparent facet
										&& currentParticleTracer.randomGenerator.rnd()>this->sh.opacity)
										|| (this->sh.reflectType > 10 //Material reflection
										&& currentParticleTracer.model->materials[this->sh.reflectType - 10].hasBackscattering //Has complex scattering
										&& currentParticleTracer.model->materials[this->sh.reflectType - 10].GetReflectionType(currentParticleTracer.energy,
										acos(Dot(currentParticleTracer.direction, this->sh.N)) - PI / 2, currentParticleTracer.randomGenerator.rnd()) == REFL_TRANS));
#endif*/
                            if (hardHit) {

                                // Hard hit
                                if (d < ray.tMax) {
                                    ray.tMax = d;
                                    //ray.lastIntersected = this->globalId;
                                    /*if(ray.hitChain->hit){
                                        ray.hitChain->next = new HitChain();
                                        ray.hitChain = ray.hitChain->next;
                                    }
                                    ray.hitChain->hit = new SimulationFacetTempVar();
                                    ray.hitChain->hit->isHit = true;
                                    ray.hitChain->hit->colU = u;
                                    ray.hitChain->hit->colV = v;
                                    ray.hitChain->hit->colDistTranspPass = d;
                                    ray.hitChain->hitId = globalId;*/

                                    ray.hardHit = HitLink(globalId, SimulationFacetTempVar(d,u,v,true));
                                    /*ray.hits.emplace_back(globalId, SimulationFacetTempVar());
                                    auto& hit = ray.hits.back().hit;
                                    hit.isHit = true;
                                    hit.colU = u;
                                    hit.colV = v;
                                    hit.colDistTranspPass = d;*/

                                }
                            }
                            else {
                                /*if(ray.hitChain->hit){
                                    ray.hitChain->next = new HitChain();
                                    ray.hitChain = ray.hitChain->next;
                                }
                                ray.hitChain->hit = new SimulationFacetTempVar();
                                ray.hitChain->hit->isHit = false;
                                ray.hitChain->hit->colU = u;
                                ray.hitChain->hit->colV = v;
                                ray.hitChain->hit->colDistTranspPass = d;
                                ray.hitChain->hitId = globalId;*/

                                ray.hits.emplace_back(globalId, SimulationFacetTempVar(d,u,v,false));
                                /*auto& hit = ray.hits.back().hit;
                                hit.isHit = false;
                                hit.colU = u;
                                hit.colV = v;
                                hit.colDistTranspPass = d;*/
                            }
                            return hardHit;
                        } // IsInFacet
                    } // d range
                } // u range
            } // v range
        } // det==0
    } // dot<0

    return false;
}