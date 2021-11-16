//
// Created by pascal on 4/22/21.
//

#include "BoundingBox.h"

#include <limits>
AxisAlignedBoundingBox::AxisAlignedBoundingBox() : min(std::numeric_limits<double>::max()),
                           max(std::numeric_limits<double>::lowest())
{};

Vector3d AxisAlignedBoundingBox::Offset(const Vector3d &p) const {
    Vector3d o = p - min;
    if (max.x > min.x) o.x /= max.x - min.x;
    if (max.y > min.y) o.y /= max.y - min.y;
    if (max.z > min.z) o.z /= max.z - min.z;
    return o;
}

Vector3d AxisAlignedBoundingBox::Diagonal() const {
    return max - min;
}

int AxisAlignedBoundingBox::MaximumExtent() const {
    Vector3d d = Diagonal();
    if (d.x > d.y && d.x > d.z)
        return 0;
    else if (d.y > d.z)
        return 1;
    else
        return 2;
}

double AxisAlignedBoundingBox::SurfaceArea() const {
    Vector3d d = Diagonal();
    return 2.0 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

AxisAlignedBoundingBox AxisAlignedBoundingBox::Union(const AxisAlignedBoundingBox& bb1, const AxisAlignedBoundingBox& bb2) {
    AxisAlignedBoundingBox unionbox;
    unionbox.min.x = std::min(bb1.min.x, bb2.min.x);
    unionbox.min.y = std::min(bb1.min.y, bb2.min.y);
    unionbox.min.z = std::min(bb1.min.z, bb2.min.z);
    unionbox.max.x = std::max(bb1.max.x, bb2.max.x);
    unionbox.max.y = std::max(bb1.max.y, bb2.max.y);
    unionbox.max.z = std::max(bb1.max.z, bb2.max.z);
    return unionbox;
}

AxisAlignedBoundingBox AxisAlignedBoundingBox::Union(const AxisAlignedBoundingBox& bb, const Vector3d& p) {
    AxisAlignedBoundingBox unionbox;
    unionbox.min.x = std::min(bb.min.x, p.x);
    unionbox.min.y = std::min(bb.min.y, p.y);
    unionbox.min.z = std::min(bb.min.z, p.z);
    unionbox.max.x = std::max(bb.max.x, p.x);
    unionbox.max.y = std::max(bb.max.y, p.y);
    unionbox.max.z = std::max(bb.max.z, p.z);
    return unionbox;
}
Vector3d &AxisAlignedBoundingBox::operator[](int ext) {
    if(ext == 0) return min;
    else return max;
}

const Vector3d &AxisAlignedBoundingBox::operator[](int ext) const {
    if(ext == 0) return min;
    else return max;
}

constexpr double machEps =
        std::numeric_limits<double>::epsilon() * 0.5;

constexpr double gamma(int n)
{
    return (n * machEps) / (1 - n * machEps);
}

bool AxisAlignedBoundingBox::IntersectBox(const Ray &ray, const Vector3d &invDir,
                                                         const int dirIsNeg[3]) const {
    double tNear, tFar;
    //X component
    double intersection1 = (min.x - ray.origin.x) * invDir.x;
    double intersection2 = (max.x - ray.origin.x) * invDir.x;
    tNear = std::min(intersection1, intersection2);
    tFar = std::max(intersection1, intersection2);
    if (tFar < 0.0) return false;

    intersection1 = (min.y - ray.origin.y) * invDir.y;
    intersection2 = (max.y - ray.origin.y) * invDir.y;
    tNear = std::max(tNear, std::min(intersection1, intersection2));
    tFar = std::min(tFar, std::max(intersection1, intersection2));
    if (tNear>tFar || tFar<0.0) return false;

    intersection1 = (min.z - ray.origin.z) * invDir.z;
    intersection2 = (max.z - ray.origin.z) * invDir.z;
    tNear = std::max(tNear, std::min(intersection1, intersection2));
    tFar = std::min(tFar, std::max(intersection1, intersection2));
    if (tNear>tFar || tFar<0.0) return false;

    return true;
}

/*bool AxisAlignedBoundingBox::IntersectBox(const Ray &ray, const Vector3d &invDir,
                                   const int dirIsNeg[3]) const {
    const AxisAlignedBoundingBox &bounds = *this;

    // Check for ray intersection against $x$ and $y$ slabs
    double tMin = (bounds[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
    double tMax = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x;
    double tyMin = (bounds[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
    double tyMax = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y;

    // Update _tMax_ and _tyMax_ to ensure robust bounds intersection
    tMax *= 1 + 2 * gamma(3);
    tyMax *= 1 + 2 * gamma(3);
    if (tMin > tyMax || tyMin > tMax) return false;
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    // Check for ray intersection against $z$ slab
    double tzMin = (bounds[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
    double tzMax = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z;

    // Update _tzMax_ to ensure robust bounds intersection
    tzMax *= 1 + 2 * gamma(3);
    if (tMin > tzMax || tzMin > tMax) return false;
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;
    return (tMin < ray.tMax) && (tMax > 0);
}*/

bool AxisAlignedBoundingBox::IntersectP(const Ray &ray, double *hitt0,
                                   double *hitt1) const {
    double t0 = 0, t1 = ray.tMax;
    for (int i = 0; i < 3; ++i) {
        // Update interval for _i_th bounding box slab
        double invRayDir = 1 / ray.direction[i];
        double tNear = (min[i] - ray.origin[i]) * invRayDir;
        double tFar = (max[i] - ray.origin[i]) * invRayDir;

        // Update parametric interval from slab intersection $t$ values
        if (tNear > tFar) std::swap(tNear, tFar);

        // Update _tFar_ to ensure robust ray--bounds intersection
        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;
    return true;
}

bool AxisAlignedBoundingBox::IntersectP(const Ray &ray, double *hitt0,
                                        double *hitt1, int dim) const {
    double t0 = 0, t1 = ray.tMax;
    {
        // Update interval for _i_th bounding box slab
        double invRayDir = 1 / ray.direction[dim];
        double tNear = (min[dim] - ray.origin[dim]) * invRayDir;
        double tFar = (max[dim] - ray.origin[dim]) * invRayDir;

        // Update parametric interval from slab intersection $t$ values
        if (tNear > tFar) std::swap(tNear, tFar);

        // Update _tFar_ to ensure robust ray--bounds intersection
        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
    }
    if (hitt0) *hitt0 = t0;
    if (hitt1) *hitt1 = t1;
    return true;
}

bool AxisAlignedBoundingBox::IsInside(const Vector3d &point) const {

    return (min[0] < point.x && max[0] > point.x)
        && (min[1] < point.y && max[1] > point.y)
        && (min[2] < point.z && max[2] > point.z);
}
