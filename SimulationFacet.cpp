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

#include "SimulationFacet.h"
#include <Polygon.h>
#include <Helper/MathTools.h>

/**
* \brief Constructor for cereal initialization
*/
SimulationFacet::SimulationFacet() : Facet() {
    isReady = false;
    globalId = 0;
    isHit = false;
}

/**
* \brief Constructor with initialisation based on the number of indices/facets
* \param nbIndex number of indices/facets
*/
SimulationFacet::SimulationFacet(size_t nbIndex) : Facet(nbIndex) {
    isReady = false;
    globalId = 0;
    isHit = false;
    indices.resize(nbIndex);                    // Ref to Geometry Vector3d
    vertices2.resize(nbIndex);
}

SimulationFacet::SimulationFacet(const SimulationFacet& cpy)  : Facet(cpy) {
    *this = cpy;
}

SimulationFacet::SimulationFacet(SimulationFacet&& cpy) noexcept : Facet(cpy){
    *this = std::move(cpy);
}

SimulationFacet& SimulationFacet::operator=(const SimulationFacet& cpy){

    this->largeEnough = cpy.largeEnough;
    this->textureCellIncrements = cpy.textureCellIncrements;
    this->sh = cpy.sh;

    isReady = cpy.isReady;
    globalId = cpy.globalId;
    indices = cpy.indices;                    // Ref to Geometry Vector3d
    vertices2 = cpy.vertices2;
    if(cpy.surf) surf = cpy.surf;
    else surf = nullptr;

    return *this;
}

SimulationFacet& SimulationFacet::operator=(SimulationFacet&& cpy) noexcept {
    this->largeEnough = std::move(cpy.largeEnough);
    this->textureCellIncrements = std::move(cpy.textureCellIncrements);
    this->sh = cpy.sh;

    isReady = cpy.isReady;
    globalId = cpy.globalId;
    indices = std::move(cpy.indices);                    // Ref to Geometry Vector3d
    vertices2 = std::move(cpy.vertices2);
    surf = cpy.surf;
    cpy.surf = nullptr;

    return *this;
}

bool SimulationFacet::InitializeLinkAndVolatile(const size_t & id)
{
    if (sh.superDest || sh.isVolatile) {
        // Link or volatile facet, overides facet settings
        // Must be full opaque and 0 sticking
        // (see SimulationMC.c::PerformBounce)
        //sh.isOpaque = true;
        sh.opacity = 1.0;
        sh.sticking = 0.0;
    }
    return true;
}



std::vector<double> SimulationFacet::InitTextureMesh()
{
	//Horrible duplicate of InterfaceFacet::BuildMesh()
	std::vector<double> interCellArea;
	try {
		interCellArea = std::vector<double>(sh.texWidth * sh.texHeight, -1.0);
	}
	catch (const std::exception& e) {
		throw Error("Couldn't allocate memory for mesh");
	}

	double sx, sy;
	double iw = 1.0 / (double)sh.texWidth_precise;
	double ih = 1.0 / (double)sh.texHeight_precise;
	double rw = sh.U.Norme() * iw;
	double rh = sh.V.Norme() * ih;
	double fullCellArea = iw * ih;

	//Construct clipping subject only once per facet
	Clipper2Lib::PathD subject(vertices2.size());
	for (int i = 0; i < vertices2.size(); i++) {
		subject[i].x = vertices2[i].u;
		subject[i].y = vertices2[i].v;
	}
	Clipper2Lib::PathsD subjects; subjects.push_back(subject);

	for (size_t j = 0; j < sh.texHeight; j++) {
		sy = (double)j;
		for (size_t i = 0; i < sh.texWidth; i++) {
			sx = (double)i;

			double u0 = sx * iw;
			double v0 = sy * ih;
			double u1 = (sx + 1.0) * iw;
			double v1 = (sy + 1.0) * ih;

			int index = j * (sh.texWidth + 1) + i;

			//intersect polygon with rectangle
			Clipper2Lib::PathD clip(4);
			clip[0] = Clipper2Lib::PointD(u0, v0, -1);
			clip[1] = Clipper2Lib::PointD(u1, v0, -1);
			clip[2] = Clipper2Lib::PointD(u1, v1, -1);
			clip[3] = Clipper2Lib::PointD(u0, v1, -1);
			Clipper2Lib::PathsD clips; clips.push_back(clip);

			std::vector<bool>visible(vertices2.size(), true); //Since SimulationFacet doesn't have 'visible' property

			auto [A, center, vList] = GetInterArea_Clipper2Lib(subjects, clips, visible);
			if (A == 0.0) { //outside the polygon
				interCellArea[i + j * sh.texWidth] = -2.0;
			}
			else if (IsEqual(fullCellArea, A, 1E-8)) { //full element
				interCellArea[i + j * sh.texWidth] = -1.0;
			} else if (A > (fullCellArea * 1.00000001)) {
				// Polyon intersection error
				// Switch back to brute force
				GLAppPolygon P2;
				P2.pts = vertices2;
				auto [bfArea, center] = GetInterAreaBF(P2, Vector2d(u0, v0), Vector2d(u1, v1));
				bool fullElem = IsZero(fullCellArea - bfArea);
				if (!fullElem) { //brute force - partial element
					interCellArea[i + j * sh.texWidth] = bfArea * (rw * rh) / (iw * ih);
				}
				else { //brute force - full element
					interCellArea[i + j * sh.texWidth] = -1.0;
				}
			}
			else { //Partial element
				// !! P1 and P2 are in u,v coordinates !!
				interCellArea[i + j * sh.texWidth] = (A * (rw * rh) / (iw * ih));
			}
		}
	}
	return interCellArea;
}

void SimulationFacet::InitializeTexture(){
    //Textures
    if (sh.isTextured) {
        size_t nbE = sh.texWidth*sh.texHeight;
        largeEnough.resize(nbE);
        // Texture increment of a full texture element
        double fullSizeInc = (sh.texWidth_precise * sh.texHeight_precise) / (sh.U.Norme() * sh.V.Norme());
        for (size_t j = 0; j < nbE; j++) { //second pass, filter out very small cells
            largeEnough[j] = textureCellIncrements[j] < (5.0*fullSizeInc);
        }
    }
}

/**
* \brief Calculates the hits size for a single facet
* \param nbMoments amount of moments
* \return calculated size of the facet hits
*/
size_t SimulationFacet::GetHitsSize(size_t nbMoments) const { //for hits dataport
    return   (1 + nbMoments)*(
            sizeof(FacetHitBuffer) +
            +(sh.isTextured ? (sh.texWidth*sh.texHeight * sizeof(TextureCell)) : 0)
            + (sh.isProfile ? (PROFILE_SIZE * sizeof(ProfileSlice)) : 0)
            + (sh.countDirection ? (sh.texWidth*sh.texHeight * sizeof(DirectionCell)) : 0)
            + sh.facetHistogramParams.GetDataSize()
    );
}

/**
* \brief Calculates the memory size for a single facet
* \param nbMoments amount of moments
* \return calculated size of the facet hits
*/
size_t SimulationFacet::GetMemSize() const {
    size_t sum = 0;
    sum += sizeof (SimulationFacet);
    sum += sizeof (size_t) * indices.capacity();
    sum += sizeof (Vector2d) * vertices2.capacity();
    sum += sizeof (double) * textureCellIncrements.capacity();
    sum += sizeof (bool) * largeEnough.capacity();
    return sum;
}