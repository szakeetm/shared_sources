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
#include "Polygon.h"
#include "GLApp/GLProgress_GUI.hpp"

#include "Clipper2Lib/include/clipper2/clipper.h"

#include <vector>
#include <list>
#include <map>
#include <GLApp/GLChart/GLChartConst.h>
#include "Buffer_shared.h"
//#include "Simulation/GeometrySimu.h"

//#define SEL_HISTORY  100
#define MAX_SUPERSTR 128
#define GEOVERSION   16

class SimulationModel;
class GlobalSimuState;
class FacetMomentSnapshot;
class SimulationFacet;

class InterfaceFacet;
class DeletedFacet;
class Worker;
class FileReader;
class FileWriter;
union PhysicalValue{
	//Unified return value that can return size_t, double or vector
	size_t count;
	double value;
	Vector3d vect;
	PhysicalValue() { new(&vect) Vector3d(); }
} ;

enum class PhysicalMode {
	CellArea,
	MCHits,
	ImpingementRate,
	ParticleDensity,
	GasDensity,
	Pressure,
	AvgGasVelocity,
	GasVelocityVector,
	NbVelocityVectors
};

class ClippingVertex {
public:

	ClippingVertex();
	Vector2d vertex; //Storing the actual vertex
	bool visited;
	bool inside{};
	bool onClippingLine{};
	bool isLink;
	double distance{};
	std::list<ClippingVertex>::iterator link;
	size_t globalId{};
};
bool operator<(const std::list<ClippingVertex>::iterator& a, const std::list<ClippingVertex>::iterator& b);

class ProjectedPoint {
public:
	Vector2d vertex2d;
	size_t globalId;
};

class UndoPoint {
public:
	Vector3d oriPos;
	size_t oriId;
};

class Geometry {
protected:
	void ResetTextureLimits(); //Different Molflow vs. Synrad
	void CalculateFacetParams(InterfaceFacet *f);
	void Merge(size_t nbV, size_t nbF, Vector3d *nV, InterfaceFacet **nF); // Merge geometry
	void LoadTXTGeom(FileReader *file, Worker* worker, size_t strIdx = 0);
	void InsertTXTGeom(FileReader *file, size_t strIdx = 0, bool newStruct = false);
	void InsertGEOGeom(FileReader *file, size_t strIdx = 0, bool newStruct = false);
	void AdjustProfile();
	void BuildDirectionList();
	void BuildSelectList();
	void BuildNonPlanarList();
	void BuildVolumeFacetList();

	float getMaxDistToCamera(InterfaceFacet* f); //unused
	int compareFacetDepth(InterfaceFacet* lhs, InterfaceFacet* rhs); //unused
public:
	Geometry();
	virtual ~Geometry();

#if defined(SYNRAD)
	virtual void ExportTextures(FILE *file, int grouping, int mode, double no_scans, GlobalSimuState &globState, bool saveSelected) {}
#endif
	virtual void BuildFacetTextures(BYTE *texture) {}

	static PhysicalValue GetPhysicalValue(InterfaceFacet* f, const PhysicalMode& mode, const double moleculesPerTP, const double densityCorrection, const double gasMass, const int index, const FacetMomentSnapshot &facetSnap); //Returns the physical value of either a facet or a texture cell
	void Clear();
	void BuildGLList();
	void InitializeInterfaceGeometry(int facet_number = -1);
    void InitializeGeometry(int facet_number = -1);           // Initialiaze all geometry related variables
    void InitializeMesh();
	void RecalcBoundingBox(int facet_number = -1);
	void CheckCollinear();
	void CheckNonSimple();
	void CheckIsolatedVertex();
	void CorrectNonSimple(int *nonSimpleList, int nbNonSimple);
	size_t AnalyzeNeigbors(Worker *work, GLProgress_Abstract& prg);
	std::vector<size_t> GetConnectedFacets(size_t sourceFacetId, double maxAngleDiff);
	std::vector<size_t> GetAllFacetIndices() const;
	size_t      GetNbFacet() const;
	size_t      GetNbVertex() const;
	Vector3d GetFacetCenter(int facet);
	size_t      GetNbStructure() const;
	char     *GetStructureName(int idx);
	GeomProperties* GetGeomProperties();
	void AddFacet(const std::vector<size_t>& vertexIds);
	void CreatePolyFromVertices_Convex(); //create convex facet from selected vertices
	void CreatePolyFromVertices_Order(); //create facet from selected vertices following selection order
	void CreateDifference(); //creates the difference from 2 selected facets. Not used anymore, superseded by ClipPolygon
	void ClipSelectedPolygons(Clipper2Lib::ClipType type, int reverseOrder);
	void ClipPolygon(size_t id1, size_t id2, Clipper2Lib::ClipType type);
	void ClipPolygon(size_t id1, std::vector<std::vector<size_t>> clippingPaths, Clipper2Lib::ClipType type);
	size_t ExecuteClip(size_t& id1,std::vector<std::vector<size_t>>& clippingPaths, std::vector<ProjectedPoint>& projectedPoints, Clipper2Lib::PolyTreeD & solution, Clipper2Lib::ClipType& type);
	void RegisterVertex(InterfaceFacet *f, const Clipper2Lib::PointD& p, size_t id1, const std::vector<ProjectedPoint> &projectedPoints, std::vector<InterfaceVertex> &newVertices, size_t registerLocation);
	void SelectCoplanar(int width, int height, double tolerance);
	InterfaceFacet    *GetFacet(size_t facet);
	InterfaceVertex *GetVertex(size_t idx);
	AxisAlignedBoundingBox     GetBB();
	Vector3d GetCenter();
    void SetPlottedFacets(std::map<int,GLColor> setMap);
    std::map<int,GLColor> GetPlottedFacets( ) const;

	// Collapsing stuff
	static int  AddRefVertex(const InterfaceVertex& p, InterfaceVertex *refs, int *nbRef, double vT);
    static int AddRefVertex(std::vector<int> &indices, std::list<InterfaceVertex> &refs, double vT);
    bool RemoveNullFacet();
	static  InterfaceFacet *MergeFacet(InterfaceFacet *f1, InterfaceFacet *f2);
	static bool GetCommonEdges(InterfaceFacet *f1, InterfaceFacet *f2, size_t * c1, size_t * c2, size_t * chainLength);
	void CollapseVertex(Worker *work, GLProgress_Abstract& prg, double totalWork, double vT);
	void RenumberNeighbors(const std::vector<int> &newRefs);
	void RenumberTeleports(const std::vector<int>& newRefs);

	void LoadTXT(FileReader *file, GLProgress_Abstract& prg, Worker* worker);
	void LoadSTR(FileReader *file, GLProgress_Abstract& prg);
	void LoadSTL(FileReader *file, GLProgress_Abstract& prg, double scaleFactor=1.0, bool insert = false, bool newStruct=false, size_t targetStructId = 0);
	void LoadASE(FileReader *file, GLProgress_Abstract& prg);

	bool IsLoaded() const;
	void InsertTXT(FileReader *file, GLProgress_Abstract& prg, bool newStr);
	void InsertGEO(FileReader *file, GLProgress_Abstract& prg, bool newStr);
	void InsertSTL(FileReader *file, GLProgress_Abstract& prg, double scaleFactor, bool newStr);

	void SaveSTR(bool saveSelected);
	void SaveSTL(FileWriter* f, GLProgress_Abstract& prg);
	void SaveSuper(int s);
	static void SaveProfileTXT(FileWriter *file);
	void UpdateSelection();
	void SwapNormal(); //Swap normals of selected facets
	void RevertFlippedNormals(); //Reverts flipping for facets with normalFlipped flag
	void SwapNormal(const std::vector < size_t> & facetList); //Swap normals of a list of facets
	void Extrude(int mode, Vector3d radiusBase, Vector3d offsetORradiusdir, bool againstNormal, double distanceORradius, double totalAngle, size_t steps);
	
	void RemoveFacets(const std::vector<size_t> &facetIdList, bool doNotDestroy = false);
	void RestoreFacets(const std::vector<DeletedFacet>& deletedFacetList, bool toEnd);
	void AddFacets(std::vector<InterfaceFacet*> facetList);
	void RemoveSelectedVertex();
	void RemoveFromStruct(int numToDel);
	void CreateLoft();
	bool RemoveCollinear();
	virtual void MoveSelectedVertex(double dX, double dY, double dZ, bool towardsDirectionMode, double distance, bool copy);
	void ScaleSelectedVertices(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker);
	void ScaleSelectedFacets(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker);
	std::vector<DeletedFacet> SplitSelectedFacets(const Vector3d &base, const Vector3d &normal, size_t *nbCreated, GLProgress_Abstract& prg);
	static bool IntersectingPlaneWithLine(const Vector3d &P0, const Vector3d &u, const Vector3d &V0, const Vector3d &n, Vector3d *intersectPoint, bool withinSection = false);
	void MoveSelectedFacets(double dX, double dY, double dZ, bool towardsDirectionMode, double distance, bool copy);
	std::vector<UndoPoint> MirrorProjectSelectedFacets(Vector3d P0, Vector3d N, bool project, bool copy, Worker *worker);
	std::vector<UndoPoint> MirrorProjectSelectedVertices(const Vector3d &P0, const Vector3d &N, bool project, bool copy, Worker *worker);
	void RotateSelectedFacets(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker);
	void RotateSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker);
	void AlignFacets(const std::vector<size_t>& memorizedSelection, size_t sourceFacetId, size_t destFacetId, size_t anchorSourceVertexId, size_t anchorDestVertexId, size_t alignerSourceVertexId, size_t alignerDestVertexId, bool invertNormal, bool invertDir1, bool invertDir2, bool copy, Worker *worker);
	int CloneSelectedFacets();
	void AddVertex(double X, double Y, double Z, bool selected = true);
	void AddVertex(const Vector3d& location, bool selected = true);
	void AddStruct(const char *name,bool deferDrawing=false);
	void DelStruct(int numToDel);
	std::vector<DeletedFacet> BuildIntersection(size_t *nbCreated);
	void    MoveVertexTo(size_t idx, double x, double y, double z);
	void	Collapse(double vT, double fT, double lT, int maxVertex, bool doSelectedOnly, Worker *work, GLProgress_Abstract& prg);
	void    SetFacetTexture(size_t facetId, double ratio, bool corrMap);
    void    SetFacetTexture(size_t facetId, double ratioU, double ratioV, bool corrMap);
    void SetFacetTextureProperties(size_t facetId, double ratioU, double ratioV, bool mesh);

    void    Rebuild();
	void    RecalcRawVertices(const int facet_number);
	void	MergecollinearSides(InterfaceFacet *f, double fT);
	void    ShiftVertex();
	int     HasIsolatedVertices();
	void    DeleteIsolatedVertices(bool selectedOnly);
	void	SelectIsolatedVertices();
	void    SetNormeRatio(float r);
	float   GetNormeRatio() const;
	void    SetAutoNorme(bool enable);
	bool    GetAutoNorme() const;
	void    SetCenterNorme(bool enable);
	bool    GetCenterNorme() const;
	void    BuildFacetList(InterfaceFacet *f);
	int		ExplodeSelected(bool toMap = false, int desType = 1, double exponent = 0.0, const double *values = NULL);

	void CreateRectangle(const Vector3d & rec_center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double  axis1Length, const double  axis2Length);
	void CreateCircle(const Vector3d & circ_center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double  axis1Length, const double  axis2Length, const size_t nbSteps);
	void CreateRacetrack(const Vector3d & race_center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double  axis1Length, const double  axis2Length, const double  topLength, const size_t nbSteps);

	void UpdateName(FileReader *file);
	std::string GetName() const;
	void UpdateName(const char *fileName);
	std::vector<size_t> GetSelectedFacets();
	std::vector<size_t> GetNonPlanarFacetIds(const double tolerance=1E-5);
	size_t GetNbSelectedFacets();
	void SetSelection(std::vector<size_t> selectedFacets, bool isShiftDown, bool isCtrlDown);
	int  RestoreDeviceObjects();
	int  InvalidateDeviceObjects();

	void     EmptyGeometry();

#pragma region GeometryRender_Shared.cpp
protected:
	void AddToSelectionHist(size_t f);
	bool AlreadySelected(size_t f);
	std::optional<size_t> GetLastSelected();
	std::optional<size_t> GetFirstSelected();
	void DrawFacetWireframe(const InterfaceFacet *f, bool offset = false, bool showHidden = false, bool selOffset = false);
	void DrawFacetWireframe_Vertexarray(const InterfaceFacet* f, std::vector<GLuint>& lines);
	void FillFacet(const InterfaceFacet *f, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord);
	void AddTextureCoord(const InterfaceFacet *f, const Vector2d &p, std::vector<float>& textureCoords);
	void DrawPolys();
	void DrawSemiTransparentPolys(const std::vector<size_t> &selectedFacets);
	void RenderArrow(GLfloat *matView, float dx, float dy, float dz, float px, float py, float pz, float d);
	void DeleteGLLists(bool deletePoly = false, bool deleteLine = false);
	void SetCullMode(int mode);
	void TriangulateForRender(const InterfaceFacet *f, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord);
	void DrawEar(const InterfaceFacet *f, const GLAppPolygon& p, int ear, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord);
public:
    bool InitOldStruct(SimulationModel* model);
    void InitInterfaceVertices(const std::vector<Vector3d>& vertices);
    virtual void InitInterfaceFacets(std::vector<std::shared_ptr<SimulationFacet>> sFacets, Worker* work);

    void SelectAll();
	void UnselectAll();
	void AddToSelectedVertexList(size_t vertexId);
	void EmptySelectedVertexList();
	void RemoveFromSelectedVertexList(size_t vertexId);
	void SelectArea(int x1, int y1, int x2, int y2, bool clear, bool unselect, bool vertexBound, bool circularSelection);
	void Select(int x, int y, bool clear, bool unselect, bool vertexBound, int width, int height);
	void TreatNewSelection(int lastFound, bool unselect);
	void SelectFacet(size_t facetId);
	void SelectAllVertex();
	void SelectVertex(int x1, int y1, int x2, int y2, bool shiftDown, bool ctrlDown, bool circularSelection, bool facetBound);
	void SelectVertex(int x, int y, int width, int height, bool shiftDown, bool ctrlDown, bool facetBound);
	void SelectVertex(int facet);
	void UnselectAllVertex();
	std::vector<size_t> GetSelectedVertices();
	size_t  GetNbSelectedVertex();
	void Render(GLfloat *matView, bool renderVolume, bool renderTexture, int showMode, bool filter, bool showHidden, bool showMesh, bool showDir);
	void RenderSemiTransparent(GLfloat *matView, bool renderVolume, bool renderTexture, int showMode, bool filter, bool showHidden, bool showMesh, bool showDir);
	void ClearFacetTextures();
	std::unordered_set<int> GetVertexBelongsToSelectedFacet();
#pragma endregion

#pragma region GeometryViewer_Shared.cpp
	void ClearFacetMeshLists();
	void BuildFacetMeshLists();
#pragma endregion
	//TEXTURE_SCALE_TYPE texture_limits[3];
    virtual bool CompareXML_simustate(const std::string &fileName_lhs, const std::string &fileName_rhs,
                                     const std::string &fileName_out, double cmpThreshold) = 0;
protected:
	// Structure viewing (-1 => all)
	GeomProperties sh;
	Vector3d  center;                     // Center (3D space)
	char      *strName[MAX_SUPERSTR];     // Structure name
	char      *strFileName[MAX_SUPERSTR]; // Structure file name
	char      strPath[512];               // Path were are stored files (super structure)

    // Geometry
	std::vector<InterfaceFacet*> facets;    // All facets of this geometry
	std::vector<InterfaceVertex> vertices3; // Vertices (3D space), can be selected
	std::vector<GLdouble> vertices_raw; //simple x,y,z coords for GL vertex array
	AxisAlignedBoundingBox bb;              // Global Axis Aligned Bounding Box (AxisAlignedBoundingBox)
	float normeRatio;     // Norme factor (direction field)
	bool  autoNorme;      // Auto normalize (direction field)
	bool  centerNorme;    // Center vector (direction field)
	bool isLoaded;  // Is loaded flag

	

	// Rendering/Selection stuff
	std::vector<size_t> selectHist;

	std::vector<size_t> selectedVertexList_ordered; //Vertex selection history, used for creating ordered polygon
	std::map<int,GLColor> plottedFacets;

	GLMATERIAL fillMaterial;
	GLMATERIAL whiteMaterial;
	GLMATERIAL arrowMaterial;
	GLint lineList[MAX_SUPERSTR]; // Compiled geometry (wire frame)
	GLint polyList;               // Compiled geometry (polygon)
	GLint selectList;             // Compiled geometry (selection)
	GLint selectList2;            // Compiled geometry (selection with offset)
	GLint selectList3;            // Compiled geometry (no offset,hidden visible)
	GLint selectHighlightList;            // Compiled geometry (no offset,hidden visible)
	GLint nonPlanarList;          // Non-planar facets with purple outline
	GLint selectListVertex;             // Compiled geometry (selection)
	GLint selectList2Vertex;            // Compiled geometry (selection with offset)
	GLint selectList3Vertex;            // Compiled geometry (no offset,hidden visible)
	GLint arrowList;              // Compiled geometry of arrow used for direction field
	GLint sphereList;             // Compiled geometry of sphere used for direction field

	public:
		bool  texAutoScale;  // Autoscale flag
		bool  texColormap;   // Colormap flag
		bool  texLogScale;   // Texture im log scale

		int viewStruct;
		int textureMode;  //MC hits/flux/power

		bool hasNonPlanar = false; //Hint for viewers to display warning label

#if defined(MOLFLOW)
#include "../src/MolflowTypes.h"
		TEXTURE_SCALE_TYPE texture_limits[3];   // Min/max values for texture scaling: Pressure/Impingement rate/Density
#endif
#if defined(SYNRAD)
    #include "../src/SynradTypes.h"
		TEXTURE_SCALE_TYPE texture_limits[3];   // Min/max values for texture scaling: Pressure/Impingement rate/Density
#endif

#if defined(SYNRAD)
		size_t loaded_nbMCHit;
		double loaded_nbHitEquiv;
		size_t loaded_nbDesorption;
		size_t loaded_desorptionLimit;
		size_t   loaded_nbLeak;
		double loaded_nbAbsEquiv;
		double loaded_distTraveledTotal;
		// Texture scaling
		TextureCell textureMin_auto, textureMin_manual, textureMax_auto,textureMax_manual;
#endif
};
