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
#include <set>
#include <list>
#include <map>
#include <GLApp/GLChart/GLChartConst.h>
#include "Buffer_shared.h"
//#include "Simulation/GeometrySimu.h"

//#define SEL_HISTORY  100
#define MAX_SUPERSTR 128
#define GEOVERSION   16

//class SimulationModel;
class GlobalSimuState;
class FacetMomentSnapshot;
class SimulationFacet;

class InterfaceFacet;
class DeletedFacet;
class Worker;
class FileReader;
class FileWriter;
union PhysicalValue{
	//Unified return value that can return int, double or vector
	int count;
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
	int globalId{};
};
bool operator<(const std::list<ClippingVertex>::iterator& a, const std::list<ClippingVertex>::iterator& b);

class ProjectedPoint {
public:
	Vector2d vertex2d;
	int globalId;
};

class UndoPoint {
public:
	Vector3d oriPos;
	int oriId;
};

class InterfaceGeometry {
protected:
	void ResetTextureLimits(); //Different Molflow vs. Synrad
	void CalculateFacetParams(InterfaceFacet *f);
	void Merge(int nbV, int nbF, Vector3d *nV, InterfaceFacet **nF); // Merge geometry
	void LoadTXTGeom(FileReader& file, Worker* worker, int strIdx = 0);
	void InsertTXTGeom(FileReader& file, int strIdx = 0, bool newStruct = false);
	void InsertGEOGeom(FileReader& file, int strIdx = 0, bool newStruct = false);
	void AdjustProfile();
	void BuildDirectionList();
	void BuildSelectList();
	void BuildNonPlanarList();
	void BuildVolumeFacetList();

	float getMaxDistToCamera(InterfaceFacet* f); //unused
	int compareFacetDepth(InterfaceFacet* lhs, InterfaceFacet* rhs); //unused
public:
	
	virtual ~InterfaceGeometry();

#if defined(SYNRAD)
	virtual void ExportTextures(FILE *file, int grouping, int mode, double no_scans, GlobalSimuState globalState, bool saveSelected) {}
#endif
	virtual void BuildFacetTextures(BYTE *texture) {}

	static PhysicalValue GetPhysicalValue(InterfaceFacet* f, const PhysicalMode& mode, const double moleculesPerTP, const double densityCorrection, const double gasMass, const int index, const FacetMomentSnapshot &facetSnap); //Returns the physical value of either a facet or a texture cell
	void Clear();
	void BuildGLList();
	void InitializeInterfaceGeometry(int facet_number = -1);
    void InitializeGeometry(int facet_number = -1);           // Initialiaze all geometry related variables
    //void InitializeMesh();
	void RecalcBoundingBox(int facet_number = -1);
	void CheckCollinear();
	void CheckNonSimple();
	void CheckIsolatedVertex();
	void CorrectNonSimple(int *nonSimpleList, int nbNonSimple);
	int AnalyzeNeigbors(Worker *work, GLProgress_Abstract& prg);
	std::vector<int> GetConnectedFacets(int sourceFacetId, double maxAngleDiff);
	std::vector<int> GetAllFacetIndices() const;
	int      GetNbFacet() const;
	int      GetNbVertex() const;
	Vector3d GetFacetCenter(int facet);
	int      GetNbStructure() const;
	std::string GetStructureName(const int idx);
	void SetStructureName(const int idx, const std::string& name);
	GeomProperties* GetGeomProperties();
	void AddFacet(const std::vector<int>& vertexIds);
	void CreatePolyFromVertices_Convex(); //create convex facet from selected vertices
	void CreatePolyFromVertices_Order(); //create facet from selected vertices following selection order
	void CreateDifference(); //creates the difference from 2 selected facets. Not used anymore, superseded by ClipPolygon
	void ClipSelectedPolygons(Clipper2Lib::ClipType type, int reverseOrder);
	void ClipPolygon(int id1, int id2, Clipper2Lib::ClipType type);
	void ClipPolygon(int id1, std::vector<std::vector<int>> clippingPaths, Clipper2Lib::ClipType type);
	int ExecuteClip(int& id1,std::vector<std::vector<int>>& clippingPaths, std::vector<ProjectedPoint>& projectedPoints, Clipper2Lib::PolyTreeD & solution, Clipper2Lib::ClipType& type);
	void RegisterVertex(InterfaceFacet *f, const Clipper2Lib::PointD& p, int id1, const std::vector<ProjectedPoint> &projectedPoints, std::vector<InterfaceVertex> &newVertices, int registerLocation);
	void SelectCoplanar(int width, int height, double tolerance);
	InterfaceFacet    *GetFacet(int facet);
	InterfaceVertex *GetVertex(int idx);
	AxisAlignedBoundingBox     GetBB();
	Vector3d GetCenter();
    void SetPlottedFacets(std::map<int,GLColor> setMap);
    std::map<int,GLColor> GetPlottedFacets( ) const;

	// Collapsing stuff
	static int  AddRefVertex(const InterfaceVertex& p, InterfaceVertex *refs, int *nbRef, double vT);
    static int AddRefVertex(std::vector<int> &indices, std::list<InterfaceVertex> &refs, double vT);
    bool RemoveNullFacet();
	static  InterfaceFacet *MergeFacet(InterfaceFacet *f1, InterfaceFacet *f2);
	static bool GetCommonEdges(InterfaceFacet *f1, InterfaceFacet *f2, int * c1, int * c2, int * chainLength);
	void CollapseVertex(Worker *work, GLProgress_Abstract& prg, double totalWork, double vT);
	void RenumberNeighbors(const std::vector<int> &newRefs);
	void RenumberTeleports(const std::vector<int>& newRefs);

	void LoadTXT(FileReader& file, GLProgress_Abstract& prg, Worker* worker);
	void LoadSTR(FileReader& file, GLProgress_Abstract& prg);
	void LoadSTL(const std::string& filePath, GLProgress_Abstract& prg, double scaleFactor=1.0, bool insert = false, bool newStruct=false, int targetStructId = 0);

	bool IsLoaded() const;
	void InsertTXT(FileReader& file, GLProgress_Abstract& prg, bool newStr);
	void InsertGEO(FileReader& file, GLProgress_Abstract& prg, bool newStr);
	//void InsertSTL(FileReader& file, bool binarySTL, GLProgress_Abstract& prg, double scaleFactor, bool newStr);

	void SaveSTR(bool saveSelected);
	void SaveSTL(FileWriter& file, GLProgress_Abstract& prg);
	void SaveStrStructure(int s);
	static void SaveProfileTXT(FileWriter& file);
	void UpdateSelection();
	void SwapNormal(); //Swap normals of selected facets
	void RevertFlippedNormals(); //Reverts flipping for facets with normalFlipped flag
	void SwapNormal(const std::vector < int> & facetList); //Swap normals of a list of facets
	void Extrude(int mode, Vector3d radiusBase, Vector3d offsetORradiusdir, bool againstNormal, double distanceORradius, double totalAngle, int steps);
	
	void RemoveFacets(const std::vector<int> &facetIdList, bool doNotDestroy = false);
	void RestoreFacets(const std::vector<DeletedFacet>& deletedFacetList, bool toEnd);
	void AddFacets(std::vector<InterfaceFacet*> facetList);
	void RemoveSelectedVertex();
	void RemoveFromStruct(int numToDel);
	void CreateLoft();
	bool RemoveCollinear();
	virtual void MoveSelectedVertex(double dX, double dY, double dZ, bool towardsDirectionMode, double distance, bool copy);
	void ScaleSelectedVertices(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker);
	void ScaleSelectedFacets(Vector3d invariant, double factorX, double factorY, double factorZ, bool copy, Worker *worker);
	std::vector<DeletedFacet> SplitSelectedFacets(const Vector3d &base, const Vector3d &normal, int *nbCreated, GLProgress_Abstract& prg);
	static bool IntersectingPlaneWithLine(const Vector3d &P0, const Vector3d &u, const Vector3d &V0, const Vector3d &n, Vector3d *intersectPoint, bool withinSection = false);
	void MoveSelectedFacets(double dX, double dY, double dZ, bool towardsDirectionMode, double distance, bool copy);
	std::vector<UndoPoint> MirrorProjectSelectedFacets(Vector3d P0, Vector3d N, bool project, bool copy, Worker *worker);
	std::vector<UndoPoint> MirrorProjectSelectedVertices(const Vector3d &P0, const Vector3d &N, bool project, bool copy, Worker *worker);
	void RotateSelectedFacets(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker);
	void RotateSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, bool copy, Worker *worker);
	void AlignFacets(const std::vector<int>& memorizedSelection, int sourceFacetId, int destFacetId, int anchorSourceVertexId, int anchorDestVertexId, int alignerSourceVertexId, int alignerDestVertexId, bool invertNormal, bool invertDir1, bool invertDir2, bool copy, Worker *worker);
	int CloneSelectedFacets();
	void AddVertex(double X, double Y, double Z, bool selected = true);
	void AddVertex(const Vector3d& location, bool selected = true);
	void AddStruct(const std::string& name,bool deferDrawing=false);
	void DelStruct(int numToDel);
	std::vector<DeletedFacet> BuildIntersection(int *nbCreated);
	void    MoveVertexTo(int idx, double x, double y, double z);
	void	Collapse(double vT, double fT, double lT, int maxVertex, bool doSelectedOnly, Worker *work, GLProgress_Abstract& prg);
	//void    SetFacetTexture(int facetId, double ratio, bool corrMap);
    void    SetFacetTexture(int facetId, double ratioU, double ratioV, bool corrMap);

    void    Rebuild();
	void    RecalcRawVertices(const int facet_number);
	void	MergecollinearSides(InterfaceFacet *f, double fT);
	void    ShiftVertex();
	int     GetNbIsolatedVertices();
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
	void CreateCircle(const Vector3d & circ_center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double  axis1Length, const double  axis2Length, const int nbSteps);
	void CreateRacetrack(const Vector3d & race_center, const Vector3d & axis1Dir, const Vector3d & normalDir, const double  axis1Length, const double  axis2Length, const double  topLength, const int nbSteps);

	void UpdateName(FileReader& file);
	std::string GetName() const;
	void UpdateName(const char *fileName);
	std::vector<int> GetSelectedFacets();
	std::vector<int> GetNonPlanarFacetIds(const double tolerance=1E-5);
	int GetNbSelectedFacets();
	void SetSelection(std::vector<int> selectedFacets, bool isShiftDown, bool isCtrlDown);
	int  RestoreDeviceObjects();
	int  InvalidateDeviceObjects();

	void     EmptyGeometry();

#pragma region GeometryRender_Shared.cpp
protected:
	void AddToSelectionHist(int f);
	bool AlreadySelected(int f);
	std::optional<int> GetLastSelected();
	std::optional<int> GetFirstSelected();
	void DrawFacetWireframe(const InterfaceFacet *f, bool offset = false, bool showHidden = false, bool selOffset = false);
	void DrawFacetWireframe_Vertexarray(const InterfaceFacet* f, std::vector<GLuint>& lines);
	void FillFacet(const InterfaceFacet *f, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord);
	void AddTextureCoord(const InterfaceFacet *f, const Vector2d &p, std::vector<float>& textureCoords);
	void DrawPolys();
	void DrawSemiTransparentPolys(const std::vector<int> &selectedFacets);
	void RenderArrow(GLfloat *matView, float dx, float dy, float dz, float px, float py, float pz, float d);
	void DeleteGLLists(bool deletePoly = false, bool deleteLine = false);
	void SetCullMode(int mode);
	void TriangulateForRender(const InterfaceFacet *f, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord);
	void DrawEar(const InterfaceFacet *f, const GLAppPolygon& p, int ear, std::vector<double>& vertexCoords, std::vector<double>& normalCoords, std::vector<float>& textureCoords, std::vector<float>& colorValues, const GLCOLOR& currentColor, bool addTextureCoord);
public:
    void SetInterfaceVertices(const std::vector<Vector3d>& vertices);
    virtual void SetInterfaceFacets(std::vector<std::shared_ptr<SimulationFacet>> sFacets, Worker* work);

    void SelectAll();
	void UnselectAll();
	void AddToSelectedVertexList(int vertexId);
	void EmptySelectedVertexList();
	void RemoveFromSelectedVertexList(int vertexId);
	void SelectArea(int x1, int y1, int x2, int y2, bool clear, bool unselect, bool vertexBound, bool circularSelection);
	void Select(int x, int y, bool clear, bool unselect, bool vertexBound, int width, int height);
	void TreatNewSelection(int lastFound, bool unselect);
	void SelectFacet(int facetId);
	void SelectAllVertex();
	void SelectVertex(int x1, int y1, int x2, int y2, bool shiftDown, bool ctrlDown, bool circularSelection, bool facetBound);
	void SelectVertex(int x, int y, int width, int height, bool shiftDown, bool ctrlDown, bool facetBound);
	void SelectVertex(int facet);
	void UnselectAllVertex();
	std::vector<int> GetSelectedVertices();
	int  GetNbSelectedVertex();
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
	std::vector<std::string> structNames = std::vector<std::string>(MAX_SUPERSTR);

    // Geometry
	std::vector<InterfaceFacet*> facets;    // All facets of this geometry
	std::vector<InterfaceVertex> vertices3; // Vertices (3D space), can be selected
	std::vector<GLdouble> vertices_raw_opengl; //simple x,y,z coords for GL vertex array
	AxisAlignedBoundingBox bb;              // Global Axis Aligned Bounding Box (AxisAlignedBoundingBox)
	float normeRatio=1.0f;     // Norme factor (direction field)
	bool  autoNorme=true;      // Auto normalize (direction field)
	bool  centerNorme=true;    // Center vector (direction field)
	bool isLoaded=false;  // Is loaded flag

	// Rendering/Selection stuff
	std::vector<int> selectHist;

	std::vector<int> selectedVertexList_ordered; //Vertex selection history, used for creating ordered polygon
	std::map<int,GLColor> plottedFacets;

	GLMATERIAL fillMaterial;
	GLMATERIAL whiteMaterial;
	GLMATERIAL arrowMaterial;
	GLint lineList[MAX_SUPERSTR]; // Compiled geometry (wire frame)
	GLint polyList=0;               // Compiled geometry (polygon)
	GLint selectList=0;             // Compiled geometry (selection)
	GLint selectList2=0;            // Compiled geometry (selection with offset)
	GLint selectList3=0;            // Compiled geometry (no offset,hidden visible)
	GLint selectHighlightList=0;            // Compiled geometry (no offset,hidden visible)
	GLint nonPlanarList=0;          // Non-planar facets with purple outline
	GLint selectListVertex=0;             // Compiled geometry (selection)
	GLint selectList2Vertex=0;            // Compiled geometry (selection with offset)
	GLint selectList3Vertex=0;            // Compiled geometry (no offset,hidden visible)
	GLint arrowList=0;              // Compiled geometry of arrow used for direction field
	GLint sphereList=0;             // Compiled geometry of sphere used for direction field

	public:
		bool  texAutoScale=true;  // Autoscale flag
		bool  texColormap=true;   // Colormap flag
		bool  texLogScale=true;   // Texture im log scale

		int viewStruct=-1; //all
		int textureMode
#ifdef MOLFLOW
			= 0;	//Pressure
#else
			= 1;	//Flux
#endif

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
		int loaded_nbMCHit;
		double loaded_nbHitEquiv;
		int loaded_nbDesorption;
		int loaded_desorptionLimit;
		int   loaded_nbLeak;
		double loaded_nbAbsEquiv;
		double loaded_distTraveledTotal;
		// Texture scaling
		TextureCell textureMin_auto, textureMin_manual, textureMax_auto,textureMax_manual;
#endif
};

struct STLTriangle {
	Vector3d normal;
	Vector3d v1, v2, v3;
	short attributeByteCount; //2-byte
};

struct STLBody {
	std::string name;
	std::vector<STLTriangle> triangles;
};

struct RawSTLfile {
	std::vector<STLBody> bodies;
	int triangleCount=0;
};

RawSTLfile LoadRawSTL(const std::string& filePath, GLProgress_Abstract& prg);