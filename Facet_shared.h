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
#pragma once
#include "Vector.h"
#include <vector>
#include "Buffer_shared.h" //DirectionCell
#include "File.h"
#include "PugiXML/pugixml.hpp"
using namespace pugi;
#include "GLApp/GLToolkit.h"
#include <cereal/archives/binary.hpp>

#if defined(SYNRAD)
#include "../src/SynradDistributions.h" //material, for Save, etc.
#endif

struct NeighborFacet {
	size_t id;
	double angleDiff;
};

class CellProperties {
public:
	//Old C-style array to save memory
	Vector2d* points;
	size_t nbPoints;
	double   area;     // Area of element
	float   uCenter;  // Center coordinates
	float   vCenter;  // Center coordinates
					  //int     elemId;   // Element index (MESH array)
					  //int full;
};

class FacetGroup; //forward declaration as it's the return value of Explode()

class InterfaceFacet { //Interface facet

	typedef struct {
		size_t u;
		size_t v;
		size_t width;
		size_t height;
	} TEXTURE_SELECTION;

public:

	// Constructor/Desctructor/Initialisation
	explicit InterfaceFacet(size_t nbIndex);
	~InterfaceFacet();

	//void  DetectOrientation();
	int   RestoreDeviceObjects();
	int   InvalidateDeviceObjects();
	bool  SetTexture(double width, double height, bool useMesh);
	void  glVertex2u(double u, double v);
	bool  BuildMesh();
	void  BuildMeshGLList();
	void  BuildSelElemList();
	void  UnselectElem();
	void  SelectElem(size_t u, size_t v, size_t width, size_t height);
	void  RenderSelectedElem();
	void  FillVertexArray(InterfaceVertex *v);
	size_t GetTexSwapSize(bool useColormap);
	size_t GetTexSwapSizeForRatio(double ratio, bool useColor);
	std::pair<size_t, size_t> GetNbCell();
	size_t GetNbCellForRatio(double ratio);
    std::pair<size_t, size_t> GetNbCellForRatio(double ratioU, double ratioV);
    void  SwapNormal();
	void  ShiftVertex(const int& offset = 1);
	void  InitVisibleEdge();
	size_t   GetIndex(int idx);
	size_t   GetIndex(size_t idx);
	double GetMeshArea(size_t index, bool correct2sides = false);
	size_t GetMeshNbPoint(size_t index);
	Vector2d GetMeshPoint(size_t index, size_t pointId);
	Vector2d GetMeshCenter(size_t index);
	double GetArea();
	bool  IsTXTLinkFacet();
	Vector3d GetRealCenter();
	void  UpdateFlags();
	FacetGroup Explode();

	//Different implementation within Molflow/Synrad
	size_t GetGeometrySize();
	void  LoadTXT(FileReader *file);
	void  SaveTXT(FileWriter *file);
	void  LoadGEO(FileReader *file, int version, size_t nbVertex);
	bool  IsCoplanarAndEqual(InterfaceFacet *f, double threshold);
	void  CopyFacetProperties(InterfaceFacet *f, bool copyMesh = false);

	//Different signature (and implementation)
#if defined(MOLFLOW) //Implementations in MolflowFacet.cpp
	void  ConvertOldDesorbType();
	void  LoadSYN(FileReader *file, int version, size_t nbVertex);
	void  LoadXML(pugi::xml_node f, size_t nbVertex, bool isMolflowFile, bool& ignoreSumMismatch, size_t vertexOffset = 0);
	void  SaveGEO(FileWriter *file, int idx);
	void  SaveXML_geom(pugi::xml_node f);
	size_t GetHitsSize(size_t nbMoments);
	size_t GetTexRamSize(size_t nbMoments);
    size_t GetTexRamSizeForCellNumber(int width, int height, bool useMesh, bool countDir, size_t nbMoments);
    size_t GetTexRamSizeForRatio(double ratio, size_t nbMoments);
    size_t GetTexRamSizeForRatio(double ratioU, double ratioV, size_t nbMoments);
    void  BuildTexture(const std::vector<TextureCell> &texBuffer, int textureMode, double min, double max, bool useColorMap, double dCoeff1, double dCoeff2, double dCoeff3, bool doLog, size_t m);
	double GetSmooth(int i, int j, TextureCell *texBuffer, int textureMode, double scaleF);
	void Sum_Neighbor(const int& i, const int& j, const double& weight, TextureCell *texBuffer, const int& textureMode, const double& scaleF, double *sum, double *totalWeight);
	std::string GetAngleMap(size_t formatId); //formatId: 1=CSV 2=TAB-separated
	void ImportAngleMap(const std::vector<std::vector<std::string>>& table);
	double DensityCorrection();
#endif
#if defined(SYNRAD) //Implementations in SynradFacet.cpp
	void  LoadSYN(FileReader *file, const std::vector<Material> &materials, int version, size_t nbVertex);
	void  LoadXML(pugi::xml_node f, size_t nbVertex, bool isMolflowFile, int vertexOffset);
	void  SaveSYN(FileWriter *file, const std::vector<Material> &materials, int idx, bool crashSave = false);
	size_t GetHitsSize();
	size_t GetTexRamSize();
	size_t GetTexRamSizeForRatio(double ratio) const;
    size_t GetTexRamSizeForRatio(double ratioU, double ratioV) const;
    void  BuildTexture(TextureCell *texture, const size_t& textureMode, const TextureCell& minVal, const TextureCell& maxVal, const double& no_scans, const bool& useColorMap, bool doLog, const bool& normalize = true);
	void Weigh_Neighbor(const size_t& i, const size_t& j, const double& weight, TextureCell* texture, const size_t& textureMode, const float& scaleF, double& weighedSum, double& totalWeigh);
	double GetSmooth(const int &i, const int &j, TextureCell *texture, const size_t& textureMode, const float &scaleF);
#endif


	std::vector<size_t>   indices;      // Indices (Reference to geometry vertex)
	std::vector<Vector2d> vertices2;    // Vertices (2D plane space, UV coordinates)

	//C-style arrays to save memory (textures can be huge):
	int      *cellPropertiesIds;      // -1 if full element, -2 if outside polygon, otherwise index in meshvector
	CellProperties* meshvector;
	size_t meshvectorsize;

	FacetProperties sh;
	FacetHitBuffer facetHitCache;
	FacetHistogramBuffer facetHistogramCache; //contains empty vectors when facet doesn't have it
	DirectionCell   *dirCache;    // Direction field cache

	// Normalized plane equation (ax + by + cz + d = 0)
	double a;
	double b;
	double c;
	double d;
	double planarityError;
	bool nonSimple = false; // A non simple polygon has crossing sides
	bool normalFlipped = false; // A flag that is true for concave facets where the normal has been flipped to obey the left-hand rule. We set it so the flip can be reverted
	//int sign; // +1: convex second vertex, -1: concave second vertex, 0: nin simple or null
	size_t texDimH;         // Texture dimension (a power of 2)
	size_t texDimW;         // Texture dimension (a power of 2)
    double tRatioU;       // Texture sample per unit
    double tRatioV;       // Texture sample per unit
    bool	textureVisible; //Draw the texture?
	bool  collinear;      //All vertices are on a line (non-simple)
	bool	volumeVisible;	//Draw volume?
	bool    hasMesh;     // Has texture
	bool textureError;   // Disable rendering if the texture has an error

	// GUI stuff
	std::vector<bool>  visible;         // Edge visible flag
	bool   selected;        // Selected flag
	TEXTURE_SELECTION    selectedElem;    // Selected mesh element
	GLint  glElem;          // Surface elements boundaries
	GLint  glSelElem;       // Selected surface elements boundaries
	GLint  glList;          // Geometry with texture
	GLuint glTex;           // Handle to OpenGL texture
	
	//Smart selection
	std::vector<NeighborFacet> neighbors;

#if defined(MOLFLOW)
	OutgassingMap ogMap;
    std::vector<size_t> angleMapCache; //Reading while loading then passing to dpHit
	bool hasOutgassingFile; //true if a desorption file was loaded and had info about this facet
	double totalFlux;
	double totalDose;

	//Parametric stuff
	std::string userOutgassing;
	std::string userSticking;
	std::string userOpacity;
#endif
	void SerializeForLoader(cereal::BinaryOutputArchive& outputarchive);
};

class FacetGroup {
public:
	size_t nbV;
	std::vector<InterfaceFacet*> facets;
	double originalPerAreaOutgassing; //Per-area outgassing of the exploded facet
};

class DeletedFacet {
public:
	InterfaceFacet *f;
	size_t ori_pos;
	bool replaceOri;
};
