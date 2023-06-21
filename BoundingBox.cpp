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

#include "BoundingBox.h"

#include <limits>

//! Init bounds with invalid values from numeric limits
AxisAlignedBoundingBox::AxisAlignedBoundingBox() : min(std::numeric_limits<double>::max()),
                           max(std::numeric_limits<double>::lowest())
{};

//! Get offset value by point p from AABB centroid
Vector3d AxisAlignedBoundingBox::Offset(const Vector3d &p) const {
    Vector3d o = p - min;
    if (max.x > min.x) o.x /= max.x - min.x;
    if (max.y > min.y) o.y /= max.y - min.y;
    if (max.z > min.z) o.z /= max.z - min.z;
    return o;
}

//! Get diagonal of AABB
Vector3d AxisAlignedBoundingBox::Diagonal() const {
    return max - min;
}

//! Get largest extent dimension by checking largest diagonal element
int AxisAlignedBoundingBox::MaximumExtent() const {
    Vector3d d = Diagonal();
    if (d.x > d.y && d.x > d.z)
        return 0;
    else if (d.y > d.z)
        return 1;
    else
        return 2;
}

//! Get surface area of AABB
double AxisAlignedBoundingBox::SurfaceArea() const {
    Vector3d d = Diagonal();
    return 2.0 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

//! Get union AABB from two AABB minima and maxima
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

//! Get union AABB from one AABB and one point minima and maxima
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

//! Quick access for minima [0] or maxima [1]
Vector3d &AxisAlignedBoundingBox::operator[](int ext) {
    if(ext == 0) return min;
    else return max;
}

//! Quick access for minima [0] or maxima [1] for const qualified AABB
const Vector3d &AxisAlignedBoundingBox::operator[](int ext) const {
    if(ext == 0) return min;
    else return max;
}

//! Epsilon value for error threshold
constexpr double machEps =
        std::numeric_limits<double>::epsilon() * 0.5;

//! Value based on error threshold to define robust intersection bounds
constexpr double gamma(int n)
{
    return (n * machEps) / (1 - n * machEps);
}

//! Ray-AABB intersection, given in the inverse direction of the ray and a handy array dirIsNeg that gives a factor for negative directions (dir < 0)
bool AxisAlignedBoundingBox::IntersectBox(const Ray &ray, const Vector3d &invDir,
                                   const int dirIsNeg[3]) const {
    const AxisAlignedBoundingBox &bounds = *this;
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