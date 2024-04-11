

#pragma once

#include <vector>
#include "RayTracing/Primitive.h"
#include "RayTracing/Ray.h"
#include "Vector.h"
#include "Buffer_shared.h"
#include "Random.h"

class Surface {
public:
    virtual ~Surface() = default;
    virtual bool IsHardHit(const Ray &r) {
        return true;
    };
};

class TransparentSurface : public Surface{
public:
    bool IsHardHit(const Ray &r) override {
        return false;
    };
};

class SemiTransparentSurface : public Surface{
    double opacity = 1.0;
public:
    SemiTransparentSurface(double opacity) : opacity(opacity){};
    bool IsHardHit(const Ray &r) override {
        return (r.rng->rnd() < opacity);
    };
};

#if defined(SYNRAD)
#include "../src/SynradDistributions.h"
class MaterialSurface : public Surface{
    double opacity{1.0};
    Material* mat;
    Vector3d N;
public:
    MaterialSurface(double opacity) : opacity(opacity), mat(nullptr){};
    MaterialSurface(Material* mat, double opacity) : opacity(opacity), mat(mat){};
    bool IsHardHit(const Ray &r) override;
};
#endif

/**
* \brief Polygon class (standard facet) that extends the Ray Tracing primitive containing various components for the post-processing features (hit tracking)
 */
class RTFacet : public RTPrimitive {
protected:
    RTFacet() : RTPrimitive(), sh(0){};
    RTFacet(size_t nbIndex) : RTPrimitive(), sh(nbIndex) {};

public:
    FacetProperties sh;
    std::vector<size_t> indices;          // Indices (Reference to geometry vertex)
    std::vector<Vector2d> vertices2;        // Vertices (2D plane space, UV coordinates)
    std::shared_ptr<Surface> surf;

    size_t globalId=0; //Global index (to identify when superstructures are present)

    void ComputeBB() { bb = sh.bb;};
    bool Intersect(Ray &r) override;

};
