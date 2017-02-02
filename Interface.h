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
#include "MirrorVertex.h"
#include "SplitFacet.h"
#include "BuildIntersection.h"
#include "RotateFacet.h"
#include "RotateVertex.h"
#include "FacetCoordinates.h"
#include "VertexCoordinates.h"
#include "SmartSelection.h"
#include "LoadStatus.h"
#include "SelectDialog.h"
#include "SelectTextureType.h"
#include "AlignFacet.h"
#include "AddVertex.h"

#define MAX_FORMULA 10
#define MAX_VIEW    19
#define MAX_SELECTION 19
#define MAX_RECENT  10

//GeometryViewer stuff
#define DOWN_MARGIN 25

//Common menu items
#define MENU_FILE_LOAD       101

#define MENU_FILE_SAVE       102
#define MENU_FILE_SAVEAS     103
#define MENU_FILE_INSERTGEO  110
#define MENU_FILE_INSERTGEO_NEWSTR  111
#define MENU_FILE_EXPORT_SELECTION     104

#define MENU_FILE_EXPORTPROFILES 105

#define MENU_FILE_LOADRECENT 120
#define MENU_FILE_EXIT       106

#define MENU_EDIT_TSCALING     201
#define MENU_EDIT_ADDFORMULA   202
#define MENU_EDIT_UPDATEFORMULAS 203
#define MENU_EDIT_GLOBALSETTINGS 204


#define MENU_FACET_COLLAPSE    301
#define MENU_FACET_SWAPNORMAL  302
#define MENU_FACET_SHIFTVERTEX 303
#define MENU_FACET_COORDINATES 304
#define MENU_FACET_DETAILS     306
#define MENU_FACET_REMOVESEL   307
#define MENU_FACET_EXPLODE     308
#define MENU_FACET_SELECTALL   309
#define MENU_FACET_SELECTSTICK 310
#define MENU_FACET_SELECTDES   311
#define MENU_FACET_SELECTABS   312
#define MENU_FACET_SELECTTRANS 313
#define MENU_FACET_SELECTREFL  314
#define MENU_FACET_SELECT2SIDE 315
#define MENU_FACET_SELECTTEXT  316
#define MENU_FACET_SELECTPROF  317
#define MENU_FACET_SELECTDEST  318
#define MENU_FACET_SELECTTELEPORT  319
#define MENU_FACET_SELECTVOL   320
#define MENU_FACET_SELECTERR   321
#define MENU_FACET_SELECTNONPLANAR 322
#define MENU_FACET_SELECTHITS        323
#define MENU_FACET_SELECTNOHITS_AREA 324
#define MENU_FACET_SAVESEL     325
#define MENU_FACET_LOADSEL     326
#define MENU_FACET_INVERTSEL   327
#define MENU_FACET_MOVE		   328
#define MENU_FACET_SCALE       329
#define MENU_FACET_MIRROR	   330
#define MENU_FACET_ROTATE	   331
#define MENU_FACET_ALIGN       332

#define MENU_FACET_CREATE_DIFFERENCE 340
#define MENU_FACET_CREATE_DIFFERENCE2 341
#define MENU_FACET_CREATE_UNION 342
#define MENU_FACET_CREATE_INTERSECTION 343
#define MENU_FACET_CREATE_XOR 344

#define MENU_FACET_EXTRUDE 350
#define MENU_FACET_SPLIT   351
#define MENU_FACET_LOFT          352
#define MENU_FACET_INTERSECT     353

#define MENU_TOOLS_TEXPLOTTER  401
#define MENU_TOOLS_PROFPLOTTER 402

#define MENU_SELECTION_ADDNEW             501
#define MENU_SELECTION_CLEARALL           502

#define MENU_SELECTION_MEMORIZESELECTIONS   510
#define MENU_SELECTION_SELECTIONS           540
#define MENU_SELECTION_CLEARSELECTIONS      570

#define MENU_SELECTION_SELECTFACETNUMBER 581
#define MENU_SELECTION_SMARTSELECTION 582
#define MENU_SELECTION_TEXTURETYPE    583

#define MENU_VERTEX_SELECTALL   601
#define MENU_VERTEX_UNSELECTALL 602
#define MENU_VERTEX_SELECT_ISOLATED 603
#define MENU_VERTEX_CLEAR_ISOLATED 604
#define MENU_VERTEX_CREATE_POLY_CONVEX   605
#define MENU_VERTEX_CREATE_POLY_ORDER    606
#define MENU_VERTEX_SELECT_COPLANAR   607
#define MENU_VERTEX_MOVE   608
#define MENU_VERTEX_ADD	   609
#define MENU_VERTEX_SCALE  610
#define MENU_VERTEX_MIRROR 611
#define MENU_VERTEX_ROTATE  612
#define MENU_VERTEX_REMOVE 613
#define MENU_VERTEX_COORDINATES 614

#define MENU_VIEW_STRUCTURE       700
#define MENU_VIEW_STRUCTURE_P     730
#define MENU_VIEW_NEWSTRUCT       731
#define MENU_VIEW_DELSTRUCT       732
#define MENU_VIEW_PREVSTRUCT	  733		
#define MENU_VIEW_NEXTSTRUCT	  734
#define MENU_VIEW_FULLSCREEN      735

#define MENU_VIEW_ADDNEW          736
#define MENU_VIEW_CLEARALL        737

#define MENU_VIEW_MEMORIZEVIEWS   740
#define MENU_VIEW_VIEWS           760
#define MENU_VIEW_CLEARVIEWS      780

#define MENU_TEST_PIPE0001        801
#define MENU_TEST_PIPE1           802
#define MENU_TEST_PIPE10          803
#define MENU_TEST_PIPE100         804
#define MENU_TEST_PIPE1000        805
#define MENU_TEST_PIPE10000       806

#define MENU_QUICKPIPE            810

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
	virtual void UpdateFacetHits(BOOL allRows=FALSE) {}
	//virtual void UpdateFormula() {}
	virtual BOOL EvaluateVariable(VLIST *v, Worker * w, Geometry * geom) { return FALSE; }
	//virtual BOOL AskToReset(Worker *work = NULL) { return FALSE; }

	virtual void BuildPipe(double ratio, int steps = 0) {}
	virtual void LoadFile(char *fName = NULL) {}
	virtual void InsertGeometry(BOOL newStr, char *fName = NULL) {}
	virtual void SaveFile() {}
	int FrameMove();

public:
	virtual void UpdateFacetParams(BOOL updateSelection=FALSE) {}
	virtual void SaveConfig() {}
	virtual void UpdatePlotters() {}

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
	//GLMenu        *facetMenu;

	GLButton      *facetApplyBtn;
	GLButton      *facetDetailsBtn;
	GLButton      *facetCoordBtn;
	GLButton      *facetMoreBtn; // <<Adv, used by Molflow only
	GLTitledPanel *facetPanel;
	GLList        *facetList;
	GLLabel       *facetSideLabel;
	GLTitledPanel *togglePanel;
	GLCombo       *facetSideType;
	GLLabel       *facetTLabel;
	GLTextField   *facetOpacity;
	GLLabel       *facetAreaLabel;
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
	
	void UpdateFacetlistSelected();
	
	int  GetVariable(char * name, char * prefix);
	void CreateOfTwoFacets(ClipperLib::ClipType type,BOOL reverseOrder=FALSE);
	void UpdateMeasurements();
	BOOL AskToSave();
	BOOL AskToReset(Worker *work = NULL);
	void AddStruct();
	void DeleteStruct();

	void SaveFileAs();
	BOOL AutoSave(BOOL crashSave = FALSE);
	void ResetAutoSaveTimer();
	void CheckForRecovery();

	AVIEW   views[MAX_VIEW];
	int     nbView;
	int     idView;
	int     curViewer;
	int     modeSolo;

	ASELECTION selections[MAX_SELECTION];
	int nbSelection;
	int idSelection;

	//Dialog
	FormulaSettings    *formulaSettings;
	CollapseSettings   *collapseSettings;
	MoveVertex		   *moveVertex;
	ScaleFacet         *scaleFacet;
	ScaleVertex        *scaleVertex;
	SelectDialog       *selectDialog;
	SelectTextureType  *selectTextureType;
	ExtrudeFacet	   *extrudeFacet;
	MoveFacet	  	   *moveFacet;
	MirrorFacet	       *mirrorFacet;
	MirrorVertex       *mirrorVertex;
	SplitFacet         *splitFacet;
	BuildIntersection  *buildIntersection;
	RotateFacet        *rotateFacet;
	RotateVertex	   *rotateVertex;
	AlignFacet         *alignFacet;
	AddVertex		   *addVertex;
	LoadStatus         *loadStatus;
	FacetCoordinates   *facetCoordinates;
	VertexCoordinates  *vertexCoordinates;
	SmartSelection     *smartSelection;

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
	void ResetSimulation(BOOL askConfirm=TRUE);
	void UpdateStructMenu();
	void UpdateTitle();

	void AnimateViewerChange(int next);
	void UpdateViewerPanel();

	void SelectViewer(int s);

	void Place3DViewer();
	void UpdateViewers();
	void SetFacetSearchPrg(BOOL visible, char *text);

	void DisplayCollapseDialog();
	void RenumberSelections(const std::vector<int> &newRefs);
	int  Resize(DWORD width, DWORD height, BOOL forceWindowed);

	// Formula management
	int nbFormula;
	FORMULA formulas[MAX_FORMULA];
	void ProcessFormulaButtons(GLComponent *src);
	BOOL OffsetFormula(char* expression, int offset, int filter = -1, std::vector<int> *newRefs = NULL);
	void UpdateFormula();
	void RenumberFormulas(std::vector<int> *newRefs);
	void AddFormula(GLParser *f, BOOL doUpdate = TRUE);
	void AddFormula(const char *fName, const char *formula);
	void ClearFormula();

	void ExportTextures(int grouping, int mode);
	
	// Recent files
	char *recents[MAX_RECENT];
	int  nbRecent;
	void AddRecent(char *fileName);
	void RemoveRecent(char *fileName);
	void UpdateRecentMenu();

	BOOL needsMesh;    //At least one viewer displays mesh
	BOOL needsTexture; //At least one viewer displays textures
	BOOL needsDirection; //At least one viewer displays direction vectors
	void CheckNeedsTexture();
	void DoEvents(BOOL forced = FALSE); //Used to catch button presses (check if an abort button was pressed during an operation)

protected:
	int OneTimeSceneInit_shared();
	int RestoreDeviceObjects_shared();
	int InvalidateDeviceObjects_shared();
	BOOL ProcessMessage_shared(GLComponent *src, int message);
	int  OnExit();
};