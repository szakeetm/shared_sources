#pragma once
//#include "Vector.h"
#include "Polygon.h"
#include "File.h"
//#include "GLApp/GLToolkit.h"
//#include "GLApp/GLGradient.h"
#include "GLApp/GLProgress.h"
#include "SMP.h"
#include "Shared.h"
#include "GrahamScan.h"
//#include "Region_full.h"
#include "PugiXML/pugixml.hpp"
#include <vector>
#include <sstream>
#include <list>
#include "Clipper\clipper.hpp"

#define SEL_HISTORY  100
#define MAX_SUPERSTR 128
#define GEOVERSION   15

class Facet;
class DeletedFacet;
class Worker;

struct NeighborFacet {
	size_t id;
	double angleDiff;
};

class CellProperties {
public:
	Vector2d* points;
	size_t nbPoints;
	float   area;     // Area of element
	float   uCenter;  // Center coordinates
	float   vCenter;  // Center coordinates
					  //int     elemId;   // Element index (MESH array)
					  //int full;
};

class ClippingVertex {
public:

	ClippingVertex();
	Vector2d vertex; //Storing the actual vertex
	BOOL visited;
	BOOL inside;
	BOOL onClippingLine;
	BOOL isLink;
	double distance;
	std::list<ClippingVertex>::iterator link;
	size_t globalId;
};
BOOL operator<(const std::list<ClippingVertex>::iterator& a, const std::list<ClippingVertex>::iterator& b);

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
	void CalculateFacetParam(Facet *f);
	void Merge(int nbV, int nbF, Vector3d *nV, Facet **nF); // Merge geometry
	void LoadTXTGeom(FileReader *file, size_t *nbV, size_t *nbF, InterfaceVertex **V, Facet ***F, size_t strIdx = 0);
	void InsertTXTGeom(FileReader *file, size_t *nbV, size_t *nbF, InterfaceVertex **V, Facet ***F, size_t strIdx = 0, BOOL newStruct = FALSE);
	void InsertGEOGeom(FileReader *file, size_t *nbV, size_t *nbF, InterfaceVertex **V, Facet ***F, size_t strIdx = 0, BOOL newStruct = FALSE);
	void InsertSTLGeom(FileReader *file, size_t *nbV, size_t *nbF, InterfaceVertex **V, Facet ***F, size_t strIdx = 0, double scaleFactor = 1.0, BOOL newStruct = FALSE);
	void AdjustProfile();
	void BuildShapeList();
	void BuildSelectList();
public:
	Geometry();
	~Geometry();

#ifdef MOLFLOW
	virtual void ExportTextures(FILE *f, int grouping, int mode, Dataport *dpHit, BOOL saveSelected) {}
#endif
#ifdef SYNRAD
	virtual void ExportTextures(FILE *file, int grouping, int mode, double no_scans, Dataport *dpHit, BOOL saveSelected) {}
#endif
	virtual void BuildFacetTextures(BYTE *hits) {}

	void Clear();
	void BuildGLList();
	void InitializeGeometry(int facet_number = -1);           // Initialiaze all geometry related variables
	void RecalcBoundingBox(int facet_number = -1);
	void CheckCollinear();
	void CheckNonSimple();
	void CheckIsolatedVertex();
	void CorrectNonSimple(int *nonSimpleList, int nbNonSimple);
	size_t AnalyzeNeighbors(Worker *work, GLProgress *prg);
	std::vector<size_t> GetConnectedFacets(size_t sourceFacetId, double maxAngleDiff);
	size_t      GetNbFacet();
	size_t      GetNbVertex();
	Vector3d GetFacetCenter(int facet);
	int      GetNbStructure();
	char     *GetStructureName(int idx);
	void CreatePolyFromVertices_Convex(); //create convex facet from selected vertices
	void CreatePolyFromVertices_Order(); //create facet from selected vertices following selection order
	void CreateDifference(); //creates the difference from 2 selected facets
	void ClipSelectedPolygons(ClipperLib::ClipType type, int reverseOrder);
	void ClipPolygon(size_t id1, size_t id2, ClipperLib::ClipType type);
	void ClipPolygon(size_t id1, std::vector<std::vector<size_t>> clippingPaths, ClipperLib::ClipType type);
	size_t ExecuteClip(size_t& id1,std::vector<std::vector<size_t>>& clippingPaths, std::vector<ProjectedPoint>& projectedPoints, ClipperLib::PolyTree & solution, ClipperLib::ClipType& type);
	void RegisterVertex(Facet *f, const Vector2d &vert, size_t id1, const std::vector<ProjectedPoint> &projectedPoints, std::vector<InterfaceVertex> &newVertices, size_t registerLocation);
	void SelectCoplanar(int width, int height, double tolerance);
	Facet    *GetFacet(int facet);
	InterfaceVertex *GetVertex(int idx);
	AABB     GetBB();
	Vector3d GetCenter();

	// Collapsing stuff
	int  AddRefVertex(InterfaceVertex *p, InterfaceVertex *refs, int *nbRef, double vT);
	BOOL RemoveNullFacet();
	Facet *MergeFacet(Facet *f1, Facet *f2);
	BOOL GetCommonEdges(Facet *f1, Facet *f2, int *c1, int *c2, int *chainLength);
	void CollapseVertex(Worker *work, GLProgress *prg, double totalWork, double vT);
	void RenumberNeighbors(const std::vector<int> &newRefs);

	void LoadTXT(FileReader *file, GLProgress *prg);
	void LoadSTR(FileReader *file, GLProgress *prg);
	void LoadSTL(FileReader *file, GLProgress *prg, double scaleFactor);
	void LoadASE(FileReader *file, GLProgress *prg);

	BOOL IsLoaded();
	void InsertTXT(FileReader *file, GLProgress *prg, BOOL newStr);
	void InsertGEO(FileReader *file, GLProgress *prg, BOOL newStr);
	void InsertSTL(FileReader *file, GLProgress *prg, double scaleFactor, BOOL newStr);

	void SaveSTR(Dataport *dhHit, BOOL saveSelected);
	void SaveSuper(int s);
	void SaveProfileTXT(FileWriter *file);
	void UpdateSelection();
	void SwapNormal();
	void Extrude(int mode, Vector3d radiusBase, Vector3d offsetORradiusdir, BOOL againstNormal, double distanceORradius, double totalAngle, int steps);
	void RemoveSelected();
	void RemoveFacets(const std::vector<size_t> &facetIdList, BOOL doNotDestroy = FALSE);
	void RestoreFacets(std::vector<DeletedFacet> deletedFacetList, BOOL toEnd);
	void RemoveSelectedVertex();
	void RemoveFromStruct(int numToDel);
	void CreateLoft();
	BOOL RemoveCollinear();
	int  ExplodeSelected(BOOL toMap = FALSE, int desType = 1, double exponent = 0.0, double *values = NULL);
	void MoveSelectedVertex(double dX, double dY, double dZ, BOOL copy, Worker *worker);
	void ScaleSelectedVertices(Vector3d invariant, double factorX, double factorY, double factorZ, BOOL copy, Worker *worker);
	void ScaleSelectedFacets(Vector3d invariant, double factorX, double factorY, double factorZ, BOOL copy, Worker *worker);
	std::vector<DeletedFacet> SplitSelectedFacets(const Vector3d &base, const Vector3d &normal, size_t *nbCreated,/*Worker *worker,*/GLProgress *prg = NULL);
	BOOL IntersectingPlaneWithLine(const Vector3d &P0, const Vector3d &u, const Vector3d &V0, const Vector3d &n, Vector3d *intersectPoint, BOOL withinSection = FALSE);
	void MoveSelectedFacets(double dX, double dY, double dZ, BOOL copy, Worker *worker);
	std::vector<UndoPoint> MirrorProjectSelectedFacets(Vector3d P0, Vector3d N, BOOL project, BOOL copy, Worker *worker);
	std::vector<UndoPoint> MirrorProjectSelectedVertices(const Vector3d &P0, const Vector3d &N, BOOL project, BOOL copy, Worker *worker);
	void RotateSelectedFacets(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, BOOL copy, Worker *worker);
	void RotateSelectedVertices(const Vector3d &AXIS_P0, const Vector3d &AXIS_DIR, double theta, BOOL copy, Worker *worker);
	void AlignFacets(int* selection, int nbSelected, int Facet_source, int Facet_dest, int Anchor_source, int Anchor_dest,
		int Aligner_source, int Aligner_dest, BOOL invertNormal, BOOL invertDir1, BOOL invertDir2, BOOL copy, Worker *worker);
	void CloneSelectedFacets();
	void AddVertex(double X, double Y, double Z, BOOL selected = TRUE);
	void AddVertex(const Vector3d& location, BOOL selected = TRUE);
	void AddStruct(const char *name);
	void DelStruct(int numToDel);
	std::vector<DeletedFacet> BuildIntersection(size_t *nbCreated);
	void     MoveVertexTo(int idx, double x, double y, double z);
	void Collapse(double vT, double fT, double lT, BOOL doSelectedOnly, Worker *work, GLProgress *prg);
	void     SetFacetTexture(int facet, double ratio, BOOL corrMap);
	void     Rebuild();
	void	   MergecollinearSides(Facet *f, double fT);
	void     ShiftVertex();
	int      HasIsolatedVertices();
	void     DeleteIsolatedVertices(BOOL selectedOnly);
	void	   SelectIsolatedVertices();
	void     SetNormeRatio(float r);
	float    GetNormeRatio();
	void     SetAutoNorme(BOOL enable);
	BOOL     GetAutoNorme();
	void     SetCenterNorme(BOOL enable);
	BOOL     GetCenterNorme();
	void     BuildFacetList(Facet *f);

	void UpdateName(FileReader *file);
	void UpdateName(const char *fileName);
	void GetSelection(int **selection, int *nbSel);
	void SetSelection(int **selection, int *nbSel, BOOL isShiftDown, BOOL isCtrlDown);
	int  RestoreDeviceObjects();
	int  InvalidateDeviceObjects();

#pragma region GeometryRender_Shared.cpp
protected:
	void AddToSelectionHist(int f);
	BOOL AlreadySelected(int f);
	void AddToSelectionHistVertex(int idx);
	BOOL AlreadySelectedVertex(int idx);
	void EmptySelectedVertexList();
	void RemoveFromSelectedVertexList(int vertexId);
	void AddToSelectedVertexList(int vertexId);
	void DrawFacet(Facet *f, BOOL offset = FALSE, BOOL showHidden = FALSE, BOOL selOffset = FALSE);
	void FillFacet(Facet *f, BOOL addTextureCoord);
	void AddTextureCoord(Facet *f, Vector2d *p);
	void DrawPolys();
	void RenderArrow(GLfloat *matView, float dx, float dy, float dz, float px, float py, float pz, float d);
	void DeleteGLLists(BOOL deletePoly = FALSE, BOOL deleteLine = FALSE);
	void SetCullMode(int mode);
	int  FindEar(POLYGON *p);
	void Triangulate(Facet *f, BOOL addTextureCoord);
	void DrawEar(Facet *f, POLYGON *p, int ear, BOOL addTextureCoord);
public:
	void SelectAll();
	void UnselectAll();
	void SelectArea(int x1, int y1, int x2, int y2, BOOL clear, BOOL unselect, BOOL vertexBound, BOOL circularSelection);
	void Select(int x, int y, BOOL clear, BOOL unselect, BOOL vertexBound, int width, int height);
	void Select(int facet);
	void Select(Facet *f);
	int  GetNbSelected();
	void SelectAllVertex();
	void SelectVertex(int x1, int y1, int x2, int y2, BOOL shiftDown, BOOL ctrlDown, BOOL circularSelection);
	void SelectVertex(int x, int y, BOOL shiftDown, BOOL ctrlDown);
	void SelectVertex(int facet);
	void UnselectAllVertex();
	int  GetNbSelectedVertex();
	void Render(GLfloat *matView, BOOL renderVolume, BOOL renderTexture, int showMode, BOOL filter, BOOL showHidden, BOOL showMesh, BOOL showDir);
	void ClearFacetTextures();
#pragma endregion

#pragma region GeometryViewer_Shared.cpp
	void ClearFacetMeshLists();
	void BuildFacetMeshLists();
#pragma endregion

protected:
	// Structure viewing (-1 => all)
	SHGEOM    sh;
	Vector3d  center;                     // Center (3D space)
	char      *strName[MAX_SUPERSTR];     // Structure name
	char      *strFileName[MAX_SUPERSTR]; // Structure file name
	char      strPath[512];               // Path were are stored files (super structure)

										  // Geometry
	Facet    **facets;    // All facets of this geometry
	InterfaceVertex  *vertices3; // Vertices (3D space), can be selected
	AABB bb;              // Global Axis Aligned Bounding Box (AABB)
	int nbSelected;       // Number of selected facets
	int nbSelectedVertex; // Number of selected vertex
	float normeRatio;     // Norme factor (direction field)
	BOOL  autoNorme;      // Auto normalize (direction field)
	BOOL  centerNorme;    // Center vector (direction field)
	BOOL isLoaded;  // Is loaded flag

	// Rendering/Selection stuff
	int selectHist[SEL_HISTORY];
	int nbSelectedHist;

	int selectHistVertex[SEL_HISTORY];
	int nbSelectedHistVertex;

	std::vector<int> selectedVertexList;

	GLMATERIAL fillMaterial;
	GLMATERIAL whiteMaterial;
	GLMATERIAL arrowMaterial;
	GLint lineList[MAX_SUPERSTR]; // Compiled geometry (wire frame)
	GLint polyList;               // Compiled geometry (polygon)
	GLint selectList;             // Compiled geometry (selection)
	GLint selectList2;            // Compiled geometry (selection with offset)
	GLint selectList3;            // Compiled geometry (no offset,hidden visible)
	GLint selectListVertex;             // Compiled geometry (selection)
	GLint selectList2Vertex;            // Compiled geometry (selection with offset)
	GLint selectList3Vertex;            // Compiled geometry (no offset,hidden visible)
	GLint arrowList;              // Compiled geometry of arrow used for direction field
	GLint sphereList;             // Compiled geometry of sphere used for direction field

	public:
		llong loaded_nbHit;
		llong loaded_nbDesorption;
		llong loaded_desorptionLimit;
		llong   loaded_nbLeak;
		llong loaded_nbAbsorption;
		double loaded_distTraveledTotal;

		BOOL  texAutoScale;  // Autoscale flag
		BOOL  texColormap;   // Colormap flag
		BOOL  texLogScale;   // Texture im log scale

		int viewStruct;
		int textureMode;  //MC hits/flux/power

#ifdef MOLFLOW
		TEXTURE_SCALE_TYPE texture_limits[3];   // Min/max values for texture scaling: Pressure/Impingement rate/Density
#endif

#ifdef SYNRAD
		// Texture scaling
		double  texMin_flux, texMin_power;        // User min
		double  texMax_flux, texMax_power;        // User max
		double  texCMin_flux, texCMin_power;       // Current minimum
		double  texCMax_flux, texCMax_power;       // Current maximum
		llong texMin_MC, texMax_MC, texCMin_MC, texCMax_MC;
#endif
};