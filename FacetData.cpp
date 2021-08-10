//
// Created by pbahr on 08/02/2021.
//

#include "FacetData.h"
#include "Polygon.h"
#include "Helper/MathTools.h"
#include "RayTracing/RTHelper.h" // SubProcessFacetTempVar
#include "RayTracing/Ray.h" // hitlink

bool Facet::Intersect(Ray &ray) {
    //++iSCount;
    nbTraversalSteps += ray.traversalSteps;
    ++nbTests;
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
                                        && currentParticle.model->materials[this->sh.reflectType - 10].hasBackscattering //Has complex scattering
                                        && currentParticle.model->materials[this->sh.reflectType - 10].GetReflectionType(currentParticle.energy,
                                                                                                                         acos(Dot(currentParticle.direction, this->sh.N)) - PI / 2, currentParticle.randomGenerator.rnd()) == REFL_TRANS));

#endif*/
                            /*#if defined(MOLFLOW)
                            double time = ray.time + d / 100.0 / currentParticle.velocity;
                            double currentOpacity = currentParticle.model->GetOpacityAt(f, time);
                            hardHit = ((currentOpacity == 1.0) || (currentParticle.randomGenerator.rnd()<currentOpacity));
#endif

#if defined(SYNRAD)
                            hardHit = !((this->sh.opacity < 0.999999 //Partially transparent facet
										&& currentParticle.randomGenerator.rnd()>this->sh.opacity)
										|| (this->sh.reflectType > 10 //Material reflection
										&& currentParticle.model->materials[this->sh.reflectType - 10].hasBackscattering //Has complex scattering
										&& currentParticle.model->materials[this->sh.reflectType - 10].GetReflectionType(currentParticle.energy,
										acos(Dot(currentParticle.direction, this->sh.N)) - PI / 2, currentParticle.randomGenerator.rnd()) == REFL_TRANS));
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
                                    ray.hitChain->hit = new SubProcessFacetTempVar();
                                    ray.hitChain->hit->isHit = true;
                                    ray.hitChain->hit->colU = u;
                                    ray.hitChain->hit->colV = v;
                                    ray.hitChain->hit->colDistTranspPass = d;
                                    ray.hitChain->hitId = globalId;*/

                                    ray.hits.emplace_back(globalId, SubProcessFacetTempVar());
                                    auto& hit = ray.hits.back().hit;
                                    hit.isHit = true;
                                    hit.colU = u;
                                    hit.colV = v;
                                    hit.colDistTranspPass = d;

                                }
                            }
                            else {
                                /*if(ray.hitChain->hit){
                                    ray.hitChain->next = new HitChain();
                                    ray.hitChain = ray.hitChain->next;
                                }
                                ray.hitChain->hit = new SubProcessFacetTempVar();
                                ray.hitChain->hit->isHit = false;
                                ray.hitChain->hit->colU = u;
                                ray.hitChain->hit->colV = v;
                                ray.hitChain->hit->colDistTranspPass = d;
                                ray.hitChain->hitId = globalId;*/

                                ray.hits.emplace_back(globalId, SubProcessFacetTempVar());
                                auto& hit = ray.hits.back().hit;
                                hit.isHit = false;
                                hit.colU = u;
                                hit.colV = v;
                                hit.colDistTranspPass = d;
                            }

                            ++nbIntersections;
                            return hardHit;
                        } // IsInFacet
                    } // d range
                } // u range
            } // v range
        } // det==0
    } // dot<0

    return false;
}