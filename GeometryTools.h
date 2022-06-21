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

#ifndef MOLFLOW_PROJ_GEOMETRYTOOLS_H
#define MOLFLOW_PROJ_GEOMETRYTOOLS_H

#include "Geometry_shared.h"
#include "NeighborScan.h"

struct NeighborFacet;

class GeometryTools {

    static int  FindEar(const GLAppPolygon& p);
    static std::vector<InterfaceFacet*> Triangulate(InterfaceFacet *f);
    //void Triangulate(Facet *f);
    static InterfaceFacet* GetTriangleFromEar(InterfaceFacet *f, const GLAppPolygon& p, int ear);
public:
    static std::vector<std::vector<NeighborFacet>> AnalyseNeighbors(Geometry* geometry);
    static std::vector<InterfaceFacet*> GetTriangulatedGeometry(Geometry* geometry, std::vector<size_t> facetIndices, GLProgress* prg = NULL);
    static void PolygonsToTriangles(Geometry* geometry);
    static void PolygonsToTriangles(Geometry* geometry, std::vector<size_t> selectedIndices);

    static int GetAnalysedCommonEdges(std::vector<Facet *> facets, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesVec(Geometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesList(std::vector<Facet *> facets, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesHash(Geometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesMap(SimulationModel *model, std::vector<CommonEdge> &commonEdges);

    static void AnalyseGeometry(const std::shared_ptr<SimulationModel>& model);

    static void CompareAlgorithm(SimulationModel *model, size_t index);

    static int GetNeighbors(Geometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesSingleVertex(Geometry *geometry, std::vector<CommonEdge> &commonEdges);
};


#endif //MOLFLOW_PROJ_GEOMETRYTOOLS_H