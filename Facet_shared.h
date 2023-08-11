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
	int id;
	double angleDiff;
};

class CellProperties {
public:
	//Old C-style array to save memory
	std::vector<Vector2d> points;
	int nbPoints;
	double   area;     // Area of element
	float   uCenter;  // Center coordinates
	float   vCenter;  // Center coordinates
					  //int     elemId;   // Element index (MESH array)
					  //int full;
};

class FacetGroup; //forward declaration as it's the return value of Explode()

class InterfaceFacet { //Interface facet

public:

	// Constructor/Desctructor/Initialisation
	explicit InterfaceFacet(int nbIndex);
	~InterfaceFacet();

	//void  DetectOrientation();
	int   RestoreDeviceObjects();
	int   InvalidateDeviceObjects();
	bool  SetTexture(double width, double height, bool useMesh);
    bool  SetTextureProperties(double width, double height);
	void  glVertex2u(double u, double v);
	void  glVertex2uVertexArray(const double u, const double v, std::vector<double>& vertexCoords) const;
	bool  BuildMesh();
	void  BuildMeshGLList();
	void  BuildSelElemList();
	void  UnselectElem();
	void  SelectElem(int u, int v, int width, int height);
	void  RenderSelectedElem();
	void  FillVertexArray(InterfaceVertex *v);
	int GetTexSwapSize(bool useColormap);
	int GetTexSwapSizeForRatio(double ratio, bool useColor);
	std::pair<int, int> GetNbCell();
	int GetNbCellForRatio(double ratio);
    std::pair<int, int> GetNbCellForRatio(double ratioU, double ratioV);
    void  SwapNormal();
	void  ShiftVertex(const int offset = 1);
	void  InitVisibleEdge();
	int   GetIndex(int idx);
	int   GetIndex(int idx);
	double GetMeshArea(int index, bool correct2sides = false);
	int GetMeshNbPoint(int index);
	Vector2d GetMeshPoint(int index, int pointId);
	Vector2d GetMeshCenter(int index);
	double GetArea();
	bool  IsTXTLinkFacet();
	Vector3d GetRealCenter();
	void  UpdateFlags();
	FacetGroup Explode();

	//Different implementation within Molflow/Synrad
	int GetGeometrySize();
	void  LoadTXT(FileReader& file);
	void  SaveTXT(FileWriter& file);
	void  LoadGEO(FileReader& file, int version, int nbVertex);
	bool  IsCoplanarAndEqual(InterfaceFacet *f, double threshold);
	void  CopyFacetProperties(InterfaceFacet *f, bool copyMesh = false);

	//Different signature (and implementation)
#if defined(MOLFLOW) //Implementations in MolflowFacet.cpp
	void  ConvertOldDesorbType();
	void  LoadSYN_facet(FileReader& file, int version, int nbVertex);
	void  LoadXML(pugi::xml_node f, int nbVertex, bool isMolflowFile, bool& ignoreSumMismatch, int vertexOffset = 0);
	void  SaveGEO(FileWriter& file, int idx);
	void  SaveXML_geom(pugi::xml_node f);
	int GetHitsSize(int nbMoments);
	int GetTexRamSize(int nbMoments);
    int GetTexRamSizeForCellNumber(int width, int height, bool useMesh, bool countDir, int nbMoments);
    int GetTexRamSizeForRatio(double ratio, int nbMoments);
    int GetTexRamSizeForRatio(double ratioU, double ratioV, int nbMoments);
    void  BuildTexture(const std::vector<TextureCell> &texBuffer, int textureMode, double min, double max, bool useColorMap, double dCoeff1, double dCoeff2, double dCoeff3, bool doLog, int m);
	double GetSmooth(int i, int j, TextureCell *texBuffer, int textureMode, double scaleF);
	void Sum_Neighbor(const int i, const int j, const double weight, TextureCell *texBuffer, const int textureMode, const double scaleF, double *sum, double *totalWeight);
	std::string GetAngleMap(int formatId); //formatId: 1=CSV 2=TAB-separated
	void ImportAngleMap(const std::vector<std::vector<std::string>>& table);
	double DensityCorrection();
#endif
#if defined(SYNRAD) //Implementations in SynradFacet.cpp
	void LoadSYN(FileReader& file, const std::vector<Material> &materials, int version, int nbVertex);
    void LoadSYNResults(FileReader& file, int version, FacetHitBuffer &facetCounter);
	void  LoadXML(pugi::xml_node f, int nbVertex, bool isMolflowFile, int vertexOffset);
	void  SaveSYN(FileWriter& file, const std::vector<Material> &materials, int idx, bool crashSave = false);
	int GetHitsSize();
	int GetTexRamSize();
	int GetTexRamSizeForRatio(double ratio) const;
    int GetTexRamSizeForRatio(double ratioU, double ratioV) const;
    void  BuildTexture(const std::vector<TextureCell> &texture, const int textureMode, const TextureCell& minVal, const TextureCell& maxVal, const double no_scans, const bool useColorMap, bool doLog, const bool normalize = true);
	void Weigh_Neighbor(const int i, const int j, const double weight, const std::vector<TextureCell> &texture, const int textureMode, const float scaleF, double& weighedSum, double& totalWeigh);
	double GetSmooth(const int i, const int j, const std::vector<TextureCell> &texture, const int textureMode, const float scaleF);
#endif


	std::vector<int>   indices;      // Indices (Reference to geometry vertex)
	std::vector<Vector2d> vertices2;    // Vertices (2D plane space, UV coordinates)

	//C-style arrays to save memory (textures can be huge):
    std::vector<int> cellPropertiesIds;      // -1 if full element, -2 if outside polygon, otherwise index in meshvector
    std::vector<CellProperties> meshvector;
	int meshvectorsize;

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
	int texDimH;         // Texture dimension (a power of 2)
	int texDimW;         // Texture dimension (a power of 2)
    double tRatioU;       // Texture sample per unit
    double tRatioV;       // Texture sample per unit

	bool  collinear;      //All vertices are on a line (non-simple)
	bool    hasMesh;     // Has texture
	bool textureError;   // Disable rendering if the texture has an error

	// GUI stuff
	std::vector<bool>  visible;         // Edge visible flag
	FacetViewSetting viewSettings;
	bool   selected;        // Selected flag

	struct TEXTURE_SELECTION{
		int u;
		int v;
		int width;
		int height;
	} ;
	TEXTURE_SELECTION    selectedElem;    // Selected mesh element
	//OpenGL
	GLint  glElem;          // Surface elements boundaries
	GLint  glSelElem;       // Selected surface elements boundaries
	GLint  glList;          // Geometry with texture
	GLuint glTex;           // Handle to OpenGL texture
	
	//Smart selection
	std::vector<NeighborFacet> neighbors;

#if defined(MOLFLOW)
	OutgassingMap ogMap;
    std::vector<int> angleMapCache; //Reading while loading then passing to dpHit
	bool hasOutgassingFile; //true if a desorption file was loaded and had info about this facet

	//Parametric stuff
	std::string userOutgassing;
	std::string userSticking;
	std::string userOpacity;
#endif
	//void SerializeForLoader(cereal::BinaryOutputArchive& outputarchive);
};

class FacetGroup {
public:
	int nbV;
	std::vector<InterfaceFacet*> facets;
	double originalPerAreaOutgassing; //Per-area outgassing of the exploded facet
};

class DeletedFacet {
public:
	InterfaceFacet *f;
	int ori_pos;
	bool replaceOri;
};
