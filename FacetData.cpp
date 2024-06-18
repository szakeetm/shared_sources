#include "FacetData.h"
#include "Polygon.h"
#include "Helper/MathTools.h"
#include "RayTracing/RTHelper.h" // FacetHitDetail
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

//Performance critical! 5% of ray-tracing CPU usage
bool RTFacet::Intersect(Ray &ray) {
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
                            if (hardHit) {
                                if (d < ray.tMax) {
                                    ray.tMax = d;
                                    ray.hardHit = HitDescriptor(globalId, FacetHitDetail(d,u,v,true));
                                }
                            }
                            else {
                                ray.transparentHits.emplace_back(globalId, FacetHitDetail(d,u,v,false));
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