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

struct NeighborFacet;
struct CommonEdge {
    CommonEdge(int id, int v1, int v2) : facetId(), v1(v1), v2(v2){
        facetId.emplace_back(id);
        angle = 0.0;
        swapped = false;
    };
    CommonEdge(size_t id, size_t v1, size_t v2) : facetId(), v1(v1), v2(v2){
        facetId.emplace_back(static_cast<int>(id));
        angle = 0.0;
        swapped = false;
    };
    CommonEdge(int id1, int id2, int v1, int v2) : facetId(), v1(v1), v2(v2){
        facetId.emplace_back(id1);
        facetId.emplace_back(id2);
        angle = 0.0;
        swapped = false;
    };
    void Merge(CommonEdge& e2){
        for(auto id : e2.facetId) {
            if (std::find(facetId.begin(), facetId.end(), id) == facetId.end()) {
                // someName not in name, add it
                facetId.push_back(id);
            }
        }
        std::sort(facetId.begin(),facetId.end());
    }

    std::vector<int> facetId; // which facet owns edge
    int v1; // index vector 1
    int v2; // index vector 2
    bool swapped;
    double angle;
};

class GeometryTools {

    
    static std::vector<InterfaceFacet*> Triangulate(InterfaceFacet *f);
    //void Triangulate(Facet *f);
    static InterfaceFacet* GetTriangleFromEar(InterfaceFacet *f, const GLAppPolygon& p, int ear);
public:
    static int  FindEar(const GLAppPolygon& p);
    //static std::vector<std::vector<NeighborFacet>> AnalyzeNeighbors(InterfaceGeometry* geometry);
    static std::vector<InterfaceFacet*> GetTriangulatedGeometry(InterfaceGeometry* geometry, std::vector<size_t> facetIndices, GLProgress_Abstract& prg);
    static void PolygonsToTriangles(InterfaceGeometry* geometry, GLProgress_Abstract& prg);
    static void PolygonsToTriangles(InterfaceGeometry* geometry, std::vector<size_t> selectedIndices, GLProgress_Abstract& prg);

    static int GetAnalyzedCommonEdges(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesVec(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesList(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesHash(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesMap(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);

    static void AnalyzeGeometry(InterfaceGeometry *geometry);

    static void CompareAlgorithm(InterfaceGeometry *geometry, size_t index);

    //static int GetNeighbors(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);

    static int GetCommonEdgesSingleVertex(InterfaceGeometry *geometry, std::vector<CommonEdge> &commonEdges);
};


#endif //MOLFLOW_PROJ_GEOMETRYTOOLS_H