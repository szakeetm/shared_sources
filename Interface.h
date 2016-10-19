#pragma once

//Shared functions of the Molflow and Synrad interface

#include "GLApp/GLApp.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLMenuBar.h"
#include "GLApp/GLWindowManager.h"

#include "GeometryViewer.h"
#include "FormulaSettings.h"
#include "CollapseSettings.h"
#include "MoveVertex.h"
#include "ScaleVertex.h"
#include "ScaleFacet.h"
#include "MoveFacet.h"
#include "ExtrudeFacet.h"
#include "MirrorFacet.h"
#include "SplitFacet.h"
#include "BuildIntersection.h"
#include "RotateFacet.h"
#include "FacetCoordinates.h"
#include "VertexCoordinates.h"
#include "SmartSelection.h"
#include "LoadStatus.h"
#include "SelectDialog.h"
#include "AlignFacet.h"
#include "AddVertex.h"

#define MAX_FORMULA 10
#define MAX_VIEW    19
#define MAX_SELECTION 19
#define MAX_RECENT  10

//GeometryViewer stuff
#define DOWN_MARGIN 25

//Common menu items
#define MENU_FILE_LOAD       11

#define MENU_FILE_SAVE       13
#define MENU_FILE_SAVEAS     14
#define MENU_FILE_INSERTGEO  140
#define MENU_FILE_INSERTGEO_NEWSTR  141
#define MENU_FILE_EXPORT_SELECTION     16

#define MENU_FILE_EXPORTPROFILES 160

#define MENU_FILE_LOADRECENT 110
#define MENU_FILE_EXIT       17

#define MENU_EDIT_TSCALING     22
#define MENU_EDIT_ADDFORMULA   23
#define MENU_EDIT_UPDATEFORMULAS 24
#define MENU_EDIT_GLOBALSETTINGS 25


#define MENU_FACET_COLLAPSE    300
#define MENU_FACET_SWAPNORMAL  301
#define MENU_FACET_SHIFTVERTEX 302
#define MENU_FACET_COORDINATES 303
#define MENU_FACET_PROFPLOTTER 304
#define MENU_FACET_DETAILS     305

#define MENU_FACET_TEXPLOTTER  307
#define MENU_FACET_REMOVESEL   308
#define MENU_FACET_EXPLODE     309
#define MENU_FACET_SELECTALL   310
#define MENU_FACET_SELECTSTICK 311
#define MENU_FACET_SELECTDES   312

#define MENU_FACET_SELECTABS   314
#define MENU_FACET_SELECTTRANS 315
#define MENU_FACET_SELECTREFL  316
#define MENU_FACET_SELECT2SIDE 317
#define MENU_FACET_SELECTTEXT  318
#define MENU_FACET_SELECTPROF  319
#define MENU_FACET_SELECTDEST  320
#define MENU_FACET_SELECTTELEPORT  321
#define MENU_FACET_SELECTVOL   322
#define MENU_FACET_SELECTERR   323
#define MENU_FACET_SELECTNONPLANAR 324
#define MENU_FACET_SELECTHITS        325
#define MENU_FACET_SELECTNOHITS_AREA 326
#define MENU_FACET_SAVESEL     327
#define MENU_FACET_LOADSEL     328
#define MENU_FACET_INVERTSEL   329
#define MENU_FACET_MOVE		   330
#define MENU_FACET_SCALE       331
#define MENU_FACET_MIRROR	   332
#define MENU_FACET_ROTATE	   333
#define MENU_FACET_ALIGN       334

#define MENU_FACET_CREATE_DIFFERENCE 3360
#define MENU_FACET_CREATE_DIFFERENCE2 3361
#define MENU_FACET_CREATE_UNION 3362
#define MENU_FACET_CREATE_INTERSECTION 3363
#define MENU_FACET_CREATE_XOR 3364
#define MENU_FACET_EXTRUDE 337
#define MENU_FACET_SPLIT   338
#define MENU_FACET_LOFT          339
#define MENU_FACET_INTERSECT     340

#define MENU_SELECTION_ADDNEW             3380
#define MENU_SELECTION_CLEARALL           3390

#define MENU_SELECTION_MEMORIZESELECTIONS   3300
#define MENU_SELECTION_SELECTIONS           3400
#define MENU_SELECTION_CLEARSELECTIONS      3500

#define MENU_SELECTION_SELECTFACETNUMBER 360
#define MENU_SELECTION_ISOLATED_VERTEX 361

#define MENU_SELECTION_SMARTSELECTION 362

#define MENU_VERTEX_SELECTALL   701
#define MENU_VERTEX_UNSELECTALL 702
#define MENU_VERTEX_CLEAR_ISOLATED 703
#define MENU_VERTEX_CREATE_POLY_CONVEX   7040
#define MENU_VERTEX_CREATE_POLY_ORDER    7041
#define MENU_VERTEX_SELECT_COPLANAR   705
#define MENU_VERTEX_MOVE   706
#define MENU_VERTEX_ADD	   707
#define MENU_VERTEX_SCALE  708
#define MENU_VERTEX_REMOVE 709
#define MENU_VERTEX_COORDINATES 710

#define MENU_VIEW_STRUCTURE       4000
#define MENU_VIEW_STRUCTURE_P     40
#define MENU_VIEW_NEWSTRUCT       401
#define MENU_VIEW_DELSTRUCT       402
#define MENU_VIEW_PREVSTRUCT	  403		
#define MENU_VIEW_NEXTSTRUCT	  404
#define MENU_VIEW_FULLSCREEN      41

#define MENU_VIEW_ADDNEW          431
#define MENU_VIEW_CLEARALL        432

#define MENU_VIEW_MEMORIZEVIEWS   4300
#define MENU_VIEW_VIEWS           4400
#define MENU_VIEW_CLEARVIEWS      4500

#define MENU_TEST_PIPE0001        60
#define MENU_TEST_PIPE1           61
#define MENU_TEST_PIPE10          62
#define MENU_TEST_PIPE100         63
#define MENU_TEST_PIPE1000        64
#define MENU_TEST_PIPE10000       65

#define MENU_QUICKPIPE            66

static const GLfloat position[] = { -0.3f, 0.3f, -1.0f, 0.0f }; //light1
static const GLfloat positionI[] = { 1.0f,-0.5f,  -0.2f, 0.0f }; //light2

static const char *fileSelFilters = "Selection files\0*.sel\0All files\0*.*\0";
static const char *fileTexFilters = "Text files\0*.txt\0All files\0*.*\0";
static const char *fileProfFilters = "CSV file\0*.csv\0Text files\0*.txt\0All files\0*.*\0";

typedef struct {
	GLLabel     *name;
	GLTextField *value;
	GLButton    *setBtn;
	GLParser    *parser;
} FORMULA;



class Interface : public GLApplication {
protected:
	Interface();
	virtual void PlaceComponents() {}
	virtual void UpdateFacetParams(BOOL updateSelection) {}
	virtual void UpdateFacetHits(BOOL allRows) {}
	//virtual void UpdateFormula() {}
	virtual BOOL EvaluateVariable(VLIST *v, Worker * w, Geometry * geom) { return FALSE; }
	virtual BOOL AskToReset(Worker *work = NULL) { return FALSE; }
public:
	// Simulation state
	float    lastUpdate;   // Last 'hit update' time
	double   hps;          // Hit per second
	double   dps;          // Desorption (or new particle) per second
	double   lastHps;      // hps measurement
	double   lastDps;      // dps measurement
	llong    lastNbHit;    // measurement
	llong    lastNbDes;    // measurement
	llong    nbDesStart;   // measurement
	llong    nbHitStart;   // measurement
	int      nbProc;       // Temporary var (use Worker::GetProcNumber)
	int      numCPU;
	float    lastAppTime;
	BOOL     antiAliasing;
	BOOL     whiteBg;
	float    lastMeasTime; // Last measurement time (for hps and dps)
	double   tolerance; //Select coplanar tolerance
	double   largeArea; //Selection filter
	double   planarityThreshold; //Planarity threshold
	int      checkForUpdates;
	int      autoUpdateFormulas;
	int      compressSavedFiles;
	int      autoSaveSimuOnly;

	BOOL     changedSinceSave; //For saving and autosaving
	double   autoSaveFrequency; //autosave period, in minutes
	float    lastSaveTime;
	float    lastSaveTimeSimu;
	std::string autosaveFilename; //only delete files that this instance saved
	BOOL     autoFrameMove; //Refresh scene every 1 second
	BOOL     frameMoveRequested; //Force frame move

	HANDLE compressProcessHandle;
	
	// Worker handle
	Worker worker;

	// Components
	GLMenuBar     *menu;
	GeometryViewer *viewer[MAX_VIEWER];
	GLTextField   *geomNumber;
	GLToggle      *showNormal;
	GLToggle      *showRule;
	GLToggle      *showUV;
	GLToggle      *showLeak;
	GLToggle      *showHit;
	GLToggle      *showLine;
	GLToggle      *showVolume;
	GLToggle      *showTexture;
	GLToggle      *showFilter;
	GLToggle      *showIndex;
	GLToggle      *showVertex;
	GLButton      *showMoreBtn;
	GLButton      *startSimu;
	GLButton      *resetSimu;

	GLCombo       *modeCombo;
	GLTextField   *hitNumber;
	GLTextField   *desNumber;
	GLTextField   *leakNumber;
	GLTextField   *sTime;
	GLMenu        *facetMenu;

	GLButton      *facetApplyBtn;
	GLButton      *facetDetailsBtn;
	GLButton      *facetCoordBtn;
	GLButton      *facetMoreBtn;
	GLTitledPanel *facetPanel;
	GLList        *facetList;
	GLLabel       *facetSideLabel;
	GLTitledPanel *togglePanel;
	GLCombo       *facetSideType;
	GLLabel       *facetTLabel;
	GLLabel       *facetTempLabel;
	GLTextField   *facetOpacity;
	GLLabel       *facetAreaLabel;
	GLTextField   *facetTemperature;
	GLTextField   *facetArea;

	GLToggle      *autoFrameMoveToggle;
	GLButton      *forceFrameMoveButton;

	GLLabel       *hitLabel;
	GLLabel       *desLabel;
	GLLabel       *leakLabel;
	GLLabel       *sTimeLabel;

	GLTitledPanel *shortcutPanel;
	GLTitledPanel *simuPanel;

	GLMenu        *structMenu;
	GLMenu        *viewsMenu;
	GLMenu        *selectionsMenu;
	GLMenu        *memorizeSelectionsMenu;
	GLMenu        *memorizeViewsMenu;
	GLMenu        *clearSelectionsMenu;
	GLMenu        *clearViewsMenu;

	// Views
	void SelectView(int v);
	void AddView(char *selectionName, AVIEW v);
	void AddView();
	void ClearViewMenus();
	void ClearAllViews();
	void OverWriteView(int idOvr);
	void ClearView(int idClr);
	void RebuildViewMenus();

	// Selections
	void SelectSelection(int v);
	void AddSelection(char *selectionName, ASELECTION s);
	void AddSelection();
	void ClearSelectionMenus();
	void ClearAllSelections();
	void OverWriteSelection(int idOvr);
	void ClearSelection(int idClr);
	void RebuildSelectionMenus();

	virtual void SaveConfig() {};

	void UpdateFacetlistSelected();
	int  GetVariable(char * name, char * prefix);
	void UpdateFormula();
	void CreateOfTwoFacets(ClipperLib::ClipType type,BOOL reverseOrder=FALSE);
	void UpdateMeasurements();
	BOOL AskToSave();

	void AddStruct();
	void DeleteStruct();

	AVIEW   views[MAX_VIEW];
	int     nbView;
	int     idView;
	int     curViewer;
	int     modeSolo;

	ASELECTION selections[MAX_SELECTION];
	int nbSelection;
	int idSelection;

	//Dialog
	FormulaSettings  *formulaSettings;
	CollapseSettings *collapseSettings;
	MoveVertex		 *moveVertex;
	ScaleFacet       *scaleFacet;
	ScaleVertex      *scaleVertex;
	SelectDialog     *selectDialog;
	ExtrudeFacet	 *extrudeFacet;
	MoveFacet		 *moveFacet;
	MirrorFacet	     *mirrorFacet;
	SplitFacet       *splitFacet;
	BuildIntersection *buildIntersection;
	RotateFacet      *rotateFacet;
	AlignFacet       *alignFacet;
	AddVertex		 *addVertex;
	LoadStatus       *loadStatus;
	FacetCoordinates *facetCoordinates;
	VertexCoordinates *vertexCoordinates;

	// Current directory
	void UpdateCurrentDir(char *fileName);
	char currentDir[1024];
	void UpdateCurrentSelDir(char *fileName);
	char currentSelDir[1024];

	// Util functions
	//void SendHeartBeat(BOOL forced=FALSE);
	char *FormatInt(llong v, char *unit);
	char *FormatPS(double v, char *unit);
	char *FormatSize(DWORD size);
	char *FormatTime(float t);
	
	void LoadSelection(char *fName = NULL);
	void SaveSelection();
	void ExportSelection();
	void UpdateModelParams();
	void UpdateViewerFlags();
	void ResetSimulation(BOOL askConfirm);
	void UpdateStructMenu();
	void UpdateTitle();

	void AnimateViewerChange(int next);
	void UpdateViewerParams();

	void SelectViewer(int s);

	void Place3DViewer();
	void UpdateViewers();
	void SetFacetSearchPrg(BOOL visible, char *text);

	void DisplayCollapseDialog();
	void RenumberSelections(const std::vector<int> &newRefs);
	void RenumberFormulas(std::vector<int> *newRefs);
	int  Resize(DWORD width, DWORD height, BOOL forceWindowed);

	// Formula management
	int nbFormula;
	FORMULA formulas[MAX_FORMULA];
	void ProcessFormulaButtons(GLComponent *src);
	BOOL OffsetFormula(char* expression, int offset, int filter = -1, std::vector<int> *newRefs = NULL);
	void AddFormula(GLParser *f, BOOL doUpdate = TRUE);
	void AddFormula(const char *fName, const char *formula);
	void ClearFormula();
	
	// Recent files
	char *recents[MAX_RECENT];
	int  nbRecent;
	void AddRecent(char *fileName);
	void RemoveRecent(char *fileName);

	BOOL needsMesh;    //At least one viewer displays mesh
	BOOL needsTexture; //At least one viewer displays textures
	void CheckNeedsTexture();

protected:
	int OneTimeSceneInit_shared();
	int RestoreDeviceObjects_shared();
	int InvalidateDeviceObjects_shared();
	void ProcessMessage_shared(GLComponent *src, int message);
	int  OnExit();
};