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

#include "SimulationModel.h"
#include "FacetData.h"
#include "SimulationFacet.h"
#include "Helper/MathTools.h"
#include <cmath>

size_t SimulationModel::GetMemSize() {
    size_t modelSize = 0;
    for (auto &fac : facets)
        modelSize += fac->GetMemSize();
    for (auto &struc : structures)
        modelSize += struc.GetMemSize();
    modelSize += sizeof(Vector3d) * vertices3.capacity();
    
    modelSize += sizeof(otfParams);
    modelSize += sizeof(sp);
    modelSize += sizeof(sh);
    modelSize += sizeof(modelMutex);
    modelSize += sizeof(initialized);

    return modelSize;
}

/**
* \brief Initialises geometry properties that haven't been loaded from file
* \return error code: 0=no error, 1=error
*/
int SimulationModel::InitializeFacets() {
    try { //unti InitializeFacets will throw error
        std::lock_guard<std::mutex> lock(modelMutex);
    }
    catch (...) {
        return 1;
    }

    for (const auto& f : facets) {
        auto& facet = *f;
        // Main facet params
        // Current facet
        //SubprocessFacet *f = model->facets[i];
        CalculateFacetParams(&facet);

        // Set some texture parameters
        // bool Facet::SetTexture(double width, double height, bool useMesh)
        if (facet.sh.texWidth_precise * facet.sh.texHeight_precise > 0.0000001) {
            const double ceilCutoff = 0.9999999;
            facet.sh.texWidth = (int) std::ceil(facet.sh.texWidth_precise *
                                                ceilCutoff); //0.9999999: cut the last few digits (convert rounding error 1.00000001 to 1, not 2)
            facet.sh.texHeight = (int) std::ceil(facet.sh.texHeight_precise * ceilCutoff);
        } else {
            facet.sh.texWidth = 0;
            facet.sh.texHeight = 0;
            facet.sh.texWidth_precise = 0.0;
            facet.sh.texHeight_precise = 0.0;
        }
    }

    return 0;
}

/*!
 * @brief Calculates various facet parameters without sanity checking @see InterfaceGeometry::CalculateFacetParams(Facet* f)
 * @param f individual subprocess facet
 */
void SimulationModel::CalculateFacetParams(RTFacet* f) {
    // Calculate facet normal
    Vector3d p0 = vertices3[f->indices[0]];
    Vector3d v1;
    Vector3d v2;
    bool consecutive = true;
    size_t ind = 2;

    // TODO: Handle possible collinear consequtive vectors
    size_t i0 = f->indices[0];
    size_t i1 = f->indices[1];
    while (ind < f->sh.nbIndex && consecutive) {
        size_t i2 = f->indices[ind++];

        v1 = vertices3[i1] - vertices3[i0]; // v1 = P0P1
        v2 = vertices3[i2] - vertices3[i1]; // v2 = P1P2
        f->sh.N = CrossProduct(v1, v2);              // Cross product
        consecutive = (f->sh.N.Norme() < 1e-3);
    }
    f->sh.N = f->sh.N.Normalized();                  // Normalize

    // Calculate Axis Aligned Bounding Box
    f->sh.bb.min = Vector3d(1e100, 1e100, 1e100);
    f->sh.bb.max = Vector3d(-1e100, -1e100, -1e100);

    for (const auto i : f->indices) {
        const Vector3d& p = vertices3[i];
        f->sh.bb.min.x = std::min(f->sh.bb.min.x,p.x);
        f->sh.bb.min.y = std::min(f->sh.bb.min.y, p.y);
        f->sh.bb.min.z = std::min(f->sh.bb.min.z, p.z);
        f->sh.bb.max.x = std::max(f->sh.bb.max.x, p.x);
        f->sh.bb.max.y = std::max(f->sh.bb.max.y, p.y);
        f->sh.bb.max.z = std::max(f->sh.bb.max.z, p.z);
    }

    // Facet center (AxisAlignedBoundingBox center)
    f->sh.center = 0.5 * (f->sh.bb.max + f->sh.bb.min);

    // Plane equation
    //double A = f->sh.N.x;
    //double B = f->sh.N.y;
    //double C = f->sh.N.z;
    //double D = -Dot(f->sh.N, p0);

    Vector3d p1 = vertices3[f->indices[1]];

    Vector3d U, V;

    U = (p1 - p0).Normalized(); //First side

    // Construct a normal vector V:
    V = CrossProduct(f->sh.N, U); // |U|=1 and |N|=1 => |V|=1

    // u,v vertices (we start with p0 at 0,0)
    f->vertices2[0].u = 0.0;
    f->vertices2[0].v = 0.0;
    Vector2d BBmin; BBmin.u = 0.0; BBmin.v = 0.0;
    Vector2d BBmax; BBmax.u = 0.0; BBmax.v = 0.0;

    for (size_t j = 1; j < f->sh.nbIndex; j++) {
        Vector3d p = vertices3[f->indices[j]];
        Vector3d v = p - p0;
        f->vertices2[j].u = Dot(U, v);  // Project p on U along the V direction
        f->vertices2[j].v = Dot(V, v);  // Project p on V along the U direction

        // Bounds
        BBmax.u  = std::max(BBmax.u , f->vertices2[j].u);
        BBmax.v = std::max(BBmax.v, f->vertices2[j].v);
        BBmin.u = std::min(BBmin.u, f->vertices2[j].u);
        BBmin.v = std::min(BBmin.v, f->vertices2[j].v);
    }

    // Calculate facet area (Meister/Gauss formula)
    double area = 0.0;
    for (size_t j = 0; j < f->sh.nbIndex; j++) {
        size_t j_next = Next(j,f->sh.nbIndex);
        area += f->vertices2[j].u*f->vertices2[j_next].v - f->vertices2[j_next].u*f->vertices2[j].v; //Equal to Z-component of vectorial product
    }
    if (area > 0.0) {

    }
    else if (area < 0.0) {
        //This is a case where a concave facet doesn't obey the right-hand rule:
        //it happens when the first rotation (usually around the second index) is the opposite as the general outline rotation

        //Do a flip
        f->sh.N = -1.0 * f->sh.N;
        V = -1.0 * V;
        BBmin.v = BBmax.v = 0.0;
        for (auto& v : f->vertices2) {
            v.v = -1.0 * v.v;
            BBmax.v = std::max(BBmax.v, v.v);
            BBmin.v = std::min(BBmin.v, v.v);
        }
    }

    f->sh.area = std::abs(0.5 * area);

    // Compute the 2D basis (O,U,V)
    double uD = (BBmax.u - BBmin.u);
    double vD = (BBmax.v - BBmin.v);

    // Origin
    f->sh.O = p0 + BBmin.u * U + BBmin.v * V;

    // Rescale U and V vector
    f->sh.nU = U;
    f->sh.U = U * uD;

    f->sh.nV = V;
    f->sh.V = V * vD;

    f->sh.Nuv = CrossProduct(f->sh.U,f->sh.V); //Not normalized normal vector

    // Rescale u,v coordinates
    for (auto& p : f->vertices2) {
        p.u = (p.u - BBmin.u) / uD;
        p.v = (p.v - BBmin.v) / vD;
    }

#if defined(MOLFLOW)
    f->sh.maxSpeed = 4.0 * std::sqrt(2.0*8.31*f->sh.temperature / 0.001 / sp.gasMass);
#endif
}
