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
#include <Helper/MathTools.h>
#include "Buffer_shared.h"

FacetGeometry::FacetGeometry(size_t nbIndices) {
    nbIndex = nbIndices;
    center.x = 0.0;
    center.y = 0.0;
    center.z = 0.0;
    area = 0.0;
}

/*!
 * @brief Calculates various facet parameters without sanity checking @see Geometry::CalculateFacetParams(Facet* f)
 * @param f individual subprocess facet
 */
void FacetGeometry::CalculateFacetParams(const std::vector<Vector3d>& vertices3, const std::vector<size_t>& indices) {
    // Calculate facet normal
    Vector3d p0 = vertices3[indices[0]];
    Vector3d v1;
    Vector3d v2;
    bool consecutive = true;
    size_t ind = 2;

    // TODO: Handle possible collinear consequtive vectors
    size_t i0 = indices[0];
    size_t i1 = indices[1];
    while (ind < nbIndex && consecutive) {
        size_t i2 = indices[ind++];

        v1 = vertices3[i1] - vertices3[i0]; // v1 = P0P1
        v2 = vertices3[i2] - vertices3[i1]; // v2 = P1P2
        N = CrossProduct(v1, v2);              // Cross product
        consecutive = (N.Norme() < 1e-11);
    }
    N = N.Normalized();                  // Normalize

    // Calculate Axis Aligned Bounding Box
    bb.min = Vector3d(1e100, 1e100, 1e100);
    bb.max = Vector3d(-1e100, -1e100, -1e100);

    for (const auto& i : indices) {
        const Vector3d& p = vertices3[i];
        bb.min.x = std::min(bb.min.x,p.x);
        bb.min.y = std::min(bb.min.y, p.y);
        bb.min.z = std::min(bb.min.z, p.z);
        bb.max.x = std::max(bb.max.x, p.x);
        bb.max.y = std::max(bb.max.y, p.y);
        bb.max.z = std::max(bb.max.z, p.z);
    }

    // Facet center (AxisAlignedBoundingBox center)
    center = 0.5 * (bb.max + bb.min);

    // Plane equation
    //double A = N.x;
    //double B = N.y;
    //double C = N.z;
    //double D = -Dot(N, p0);

    Vector3d p1 = vertices3[indices[1]];

    Vector3d U, V;

    U = (p1 - p0).Normalized(); //First side

    // Construct a normal vector V:
    V = CrossProduct(N, U); // |U|=1 and |N|=1 => |V|=1

    // u,v vertices (we start with p0 at 0,0)
    vertices2[0].u = 0.0;
    vertices2[0].v = 0.0;
    Vector2d BBmin; BBmin.u = 0.0; BBmin.v = 0.0;
    Vector2d BBmax; BBmax.u = 0.0; BBmax.v = 0.0;

    for (size_t j = 1; j < nbIndex; j++) {
        Vector3d p = vertices3[indices[j]];
        Vector3d v = p - p0;
        vertices2[j].u = Dot(U, v);  // Project p on U along the V direction
        vertices2[j].v = Dot(V, v);  // Project p on V along the U direction

        // Bounds
        BBmax.u  = std::max(BBmax.u , vertices2[j].u);
        BBmax.v = std::max(BBmax.v, vertices2[j].v);
        BBmin.u = std::min(BBmin.u, vertices2[j].u);
        BBmin.v = std::min(BBmin.v, vertices2[j].v);
    }

    // Calculate facet area (Meister/Gauss formula)
    double area = 0.0;
    for (size_t j = 0; j < nbIndex; j++) {
        size_t j_next = Next(j,nbIndex);
        area += vertices2[j].u*vertices2[j_next].v - vertices2[j_next].u*vertices2[j].v; //Equal to Z-component of vectorial product
    }
    if (area > 0.0) {

    }
    else if (area < 0.0) {
        //This is a case where a concave facet doesn't obey the right-hand rule:
        //it happens when the first rotation (usually around the second index) is the opposite as the general outline rotation

        //Do a flip
        N = -1.0 * N;
        V = -1.0 * V;
        BBmin.v = BBmax.v = 0.0;
        for (auto& v : vertices2) {
            v.v = -1.0 * v.v;
            BBmax.v = std::max(BBmax.v, v.v);
            BBmin.v = std::min(BBmin.v, v.v);
        }
    }

    area = std::abs(0.5 * area);

    // Compute the 2D basis (O,U,V)
    double uD = (BBmax.u - BBmin.u);
    double vD = (BBmax.v - BBmin.v);

    // Origin
    O = p0 + BBmin.u * U + BBmin.v * V;

    // Rescale U and V vector
    nU = U;
    U = U * uD;

    nV = V;
    V = V * vD;

    Nuv = CrossProduct(U,V); //Not normalized normal vector

    // Rescale u,v coordinates
    for (auto& p : vertices2) {
        p.u = (p.u - BBmin.u) / uD;
        p.v = (p.v - BBmin.v) / vD;
    }
}

GlobalHitBuffer& GlobalHitBuffer::operator+=(const GlobalHitBuffer& src) {
    this->globalHits += src.globalHits;

    this->distTraveled_total += src.distTraveled_total;
#if defined(MOLFLOW)
    this->distTraveledTotal_fullHitsOnly += src.distTraveledTotal_fullHitsOnly;
#endif
    this->nbLeakTotal += src.nbLeakTotal;

    return *this;
}

#if defined(MOLFLOW)
/**
* \brief += operator, with simple += of underlying structures
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetHitBuffer& FacetHitBuffer::operator+=(const FacetHitBuffer& rhs) {
    this->nbDesorbed+=rhs.nbDesorbed;
    this->nbMCHit+=rhs.nbMCHit;
    this->nbHitEquiv+=rhs.nbHitEquiv;
    this->nbAbsEquiv+=rhs.nbAbsEquiv;
    this->sum_1_per_ort_velocity+=rhs.sum_1_per_ort_velocity;
    this->sum_1_per_velocity+=rhs.sum_1_per_velocity;
    this->sum_v_ort+=rhs.sum_v_ort;
    return *this;
}
#endif
#if defined(SYNRAD)
FacetHitBuffer::FacetHitBuffer(){
    this->ResetBuffer();
}

void FacetHitBuffer::ResetBuffer(){
    this->nbMCHit = 0;
    this->nbDesorbed = 0;
    this->nbHitEquiv = 0.0;
    this->nbAbsEquiv = 0.0;
    this->fluxAbs = 0.0;
    this->powerAbs = 0.0;
}

FacetHitBuffer & FacetHitBuffer::operator+=(const FacetHitBuffer & rhs){
    this->nbMCHit += rhs.nbMCHit;
    this->nbDesorbed += rhs.nbDesorbed;
    this->nbHitEquiv += rhs.nbHitEquiv;
    this->nbAbsEquiv += rhs.nbAbsEquiv;
    this->fluxAbs += rhs.fluxAbs;
	this->powerAbs += rhs.powerAbs;
	return *this;
}
void FacetHistogramBuffer::Resize(const HistogramParams& params) {
    this->nbHitsHistogram.resize(params.recordBounce ? params.GetBounceHistogramSize() : 0);
    this->nbHitsHistogram.shrink_to_fit();
    this->distanceHistogram.resize(params.recordDistance ? params.GetDistanceHistogramSize() : 0);
    this->distanceHistogram.shrink_to_fit();
}

void FacetHistogramBuffer::Reset(){
    ZEROVECTOR(nbHitsHistogram);
    ZEROVECTOR(distanceHistogram);
#if defined(MOLFLOW)
    ZEROVECTOR(timeHistogram);
#endif
}
#endif

