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

//Shared functions of the Molflow and Synrad interface
#include <list> // for recents
#include <memory>
#include <Formulas.h>

#include "Worker.h"
#include "GeometryViewer.h"

#include <GLApp/GLApp.h>
#include <GLApp/GLFormula.h>

#define USINGZ //Allows clipper library to store user data (globalId)
#include "Clipper2Lib/include/clipper2/clipper.h"

class GLTextField;
class GLToggle;
class GLLabel;
class GLButton;
class GLTitledPanel;
class GLList;
class GLCombo;
class GLMenuBar;
class GLFormula;
class GLMenu;

class GeometryViewer;
class CollapseSettings;
class HistogramSettings;
class HistogramPlotter;
class MoveVertex;
class ScaleVertex;
class ScaleFacet;
class MoveFacet;
class ExtrudeFacet;
class MirrorFacet;
class CreateShape;
class MirrorVertex;
class SplitFacet;
class BuildIntersection;
class RotateFacet;
class RotateVertex;
class FacetCoordinates;
class VertexCoordinates;
class SmartSelection;
class SelectDialog;
class SelectTextureType;
class SelectFacetByResult;
class AlignFacet;
class AddVertex;
class FormulaEditor;
class AppUpdater;
class UpdateCheckDialog;
class UpdateFoundDialog;
class UpdateLogWindow;
class ManualUpdateCheckDialog;
class ParticleLogger;
class ConvergencePlotter;
class FacetDetails;
class Viewer3DSettings;
class TextureScaling;
class GlobalSettings;
class ProfilePlotter;
class TexturePlotter;
class CrossSection;

class InterfaceGeometry;
class ImguiWindow;

#define MAX_VIEWER  4
//#define MAX_FORMULA 10
#define MAX_VIEW    19
#define MAX_RECENT  10

//GeometryViewer stuff
#define DOWN_MARGIN 25

//Common menu items
#define MENU_FILE_NEW       100
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
#define MENU_TOOLS_FORMULAEDITOR 203
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
#define MENU_FACET_SELECT_BY_RESULT 325
#define MENU_FACET_SAVESEL     326
#define MENU_FACET_LOADSEL     327
#define MENU_FACET_INVERTSEL   328
#define MENU_FACET_MOVE		   329
#define MENU_FACET_SCALE       330
#define MENU_FACET_MIRROR	   331
#define MENU_FACET_ROTATE	   332
#define MENU_FACET_ALIGN       333
#define MENU_FACET_CREATESHAPE 334
#define MENU_FACET_REVERTFLIP 335

#define MENU_FACET_CREATE_DIFFERENCE 340
#define MENU_FACET_CREATE_DIFFERENCE2 341
#define MENU_FACET_CREATE_DIFFERENCE_AUTO 342
#define MENU_FACET_CREATE_UNION 343
#define MENU_FACET_CREATE_INTERSECTION 344
#define MENU_FACET_CREATE_XOR 345

#define MENU_FACET_EXTRUDE 350
#define MENU_FACET_SPLIT   351
#define MENU_FACET_LOFT          352
#define MENU_FACET_INTERSECT     353
#define MENU_FACET_TRIANGULATE   354

#define MENU_TOOLS_TEXPLOTTER  401
#define MENU_TOOLS_PROFPLOTTER 402
#define MENU_TOOLS_PARTICLELOGGER 403
#define MENU_TOOLS_HISTOGRAMSETTINGS 404
#define MENU_TOOLS_HISTOGRAMPLOTTER 405
#define MENU_TOOLS_SCREENSHOT 406
#define MENU_TOOLS_CONVPLOTTER 407

#define MENU_SELECTION_ADDNEW             501
#define MENU_SELECTION_CLEARALL           502

#define MENU_SELECTION_MEMORIZESELECTIONS   5100
#define MENU_SELECTION_SELECTIONS           5200
#define MENU_SELECTION_CLEARSELECTIONS      5300

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

#define MENU_VIEW_STRUCTURE       7000
#define MENU_VIEW_NEWSTRUCT       731
#define MENU_VIEW_DELSTRUCT       732
#define MENU_VIEW_PREVSTRUCT	  733		
#define MENU_VIEW_NEXTSTRUCT	  734
#define MENU_VIEW_FULLSCREEN      735

#define MENU_VIEW_ADDNEW          736
#define MENU_VIEW_CLEARALL        737

#define MENU_VIEW_MEMORIZEVIEWS		740
#define MENU_VIEW_VIEWS				760
#define MENU_VIEW_CLEARVIEWS		780
#define MENU_VIEW_CROSSSECTION		790

#define MENU_TEST_PIPE0001        801
#define MENU_TEST_PIPE1           802
#define MENU_TEST_PIPE10          803
#define MENU_TEST_PIPE100         804
#define MENU_TEST_PIPE1000        805
#define MENU_TEST_PIPE10000       806
#define MENU_TEST_PIPEN           807

#define MENU_QUICKPIPE            810

#define MENU_TRIANGULATE          815
#define MENU_ANALYZE              816
#define MENU_CMP_RES              817

#define MENU_ABOUT                1000
#define MENU_UPDATE               1001
#define MENU_IMGUI                1100
#define MENU_IMGUI_GLOB           1101
#define MENU_IMGUI_SIDE           1102
#define MENU_IMGUI_MENU           1103


static const GLfloat position[] = { -0.3f, 0.3f, -1.0f, 0.0f }; //light1
static const GLfloat positionI[] = { 1.0f,-0.5f,  -0.2f, 0.0f }; //light2

constexpr size_t SmoothStatSizeLimit() { return 16; }

class Interface : public GLApplication {
protected:
	Interface();
	virtual ~Interface();
	virtual void PlaceComponents() {}
	virtual void UpdateFacetHits(bool allRows) {}
	virtual void ClearFacetParams() {}
	virtual void LoadConfig() {}
	//virtual bool AskToReset(Worker *work = NULL) { return false; }

	virtual void BuildPipe(double ratio, int steps) {};
	virtual void EmptyGeometry() {}
	virtual void LoadFile(const std::string& fileName) {}
	virtual void InsertGeometry(bool newStr, const std::string& fileName) {}
	virtual void SaveFile() {}
	int FrameMove() override;
public:
	virtual void ImLoadFromFile(const std::unique_ptr<MolflowInterfaceSettings>& interfaceSettings);
	virtual void UpdateFacetParams(bool updateSelection) {}
	virtual void SaveConfig() {}
	virtual void UpdatePlotters() {}

	void ImRefresh();
	void ImReset();
	void ImClear();

	// Simulation state
	float    lastUpdate;   // Last 'hit update' time
	template <typename T, bool useDiff = false>
	struct EventPerSecond {
		EventPerSecond(size_t limit = SmoothStatSizeLimit()) : N(limit) {};

		double avg() {
			if (eventsAtTime.empty()) return 0.0;
			auto& first = eventsAtTime.front();
			auto& last = eventsAtTime.back();

			double avg = 0.0;
			if (last.second - first.second > 0.0)
				avg = static_cast<double>(sum) / (last.second - first.second);

			if (avg != 0.0)
				lastCached = avg;
			return lastCached;
		}

		double last() {
			return lastCached;
		}

		void push(T event, double time) {
			if (!eventsAtTime.empty()) {
				if (time == eventsAtTime.back().second)
					return;
			}
			if (eventsAtTime.size() >= N) {
				if (!useDiff)
					sum -= eventsAtTime.front().first;
				eventsAtTime.pop();
			}
			eventsAtTime.push({ event, time });
			if (useDiff)
				sum = eventsAtTime.back().first - eventsAtTime.front().first;
			else
				sum += event;
		}

		void clear() {
			std::queue<std::pair<T, double>>().swap(eventsAtTime);
			sum = 0;
			lastCached = 0.0;
		}

		std::queue<std::pair<T, double>> eventsAtTime;
	private:
		T sum = 0;
		size_t N;
		double lastCached = 0.0;
	};

	EventPerSecond<size_t>   hps;          // Hit per second
	EventPerSecond<size_t>   dps;          // Hit per second
	EventPerSecond<size_t, true>   hps_runtotal{2};          // Hit per second
	EventPerSecond<size_t, true>   dps_runtotal{2};          // Hit per second

	size_t    lastNbHit;    // measurement
	size_t    lastNbDes;    // measurement
	size_t    nbDesStart;   // measurement
	size_t    nbHitStart;   // measurement
	size_t    nbProc;       // Temporary var (use Worker::GetProcNumber)
	size_t    numCPU;

	bool useOldXMLFormat = false;
	bool     antiAliasing;
	bool     whiteBg;
	bool highlightSelection;
	bool highlightNonplanarFacets;
	bool	 leftHandedView;
	float    lastMeasTime; // Last measurement time (for hps and dps)
	double   coplanarityTolerance; //Select coplanar tolerance
	double   largeAreaThreshold; //Selection filter
	double   planarityThreshold; //Planarity threshold

	AppUpdater* appUpdater;
	bool     autoUpdateFormulas;
	bool     compressSavedFiles;
	bool      autoSaveSimuOnly;

	bool     changedSinceSave; //For saving and autosaving
	double   autoSaveFrequency; //autosave period, in minutes
	float    lastSaveTime;
	float    lastSaveTimeSimu;
	std::string autosaveFilename; //only delete files that this instance saved
	bool     autoFrameMove; //Refresh scene every 1 second
	bool     updateRequested; //Force frame move
	//bool     prevRunningState; //Previous state to react for state change

	std::shared_ptr<Formulas> appFormulas;

#ifdef _WIN32
	HANDLE compressProcessHandle;
#endif

	// Worker handle
	Worker worker;

	// Components
	GLMenuBar* menu;
	GeometryViewer* viewers[MAX_VIEWER];
	GLTextField* geomNumber;
	GLToggle* showNormal;
	GLToggle* showRule;
	GLToggle* showUV;
	GLToggle* showLeak;
	GLToggle* showHit;
	GLToggle* showLine;
	GLToggle* showVolume;
	GLToggle* showTexture;
	GLToggle* showFacetId;
	GLToggle* showIndex;
	GLToggle* showVertexId;
	GLButton* viewerMoreButton;
	GLCombo* physicsModeCombo;

	GLButton* globalSettingsBtn;
	GLButton* startSimu;
	GLButton* resetSimu;

	GLTextField* hitNumber;
	GLTextField* desNumber;
	GLTextField* leakNumber;
	GLTextField* sTime;
	//GLMenu        *facetMenu;

	GLButton* facetApplyBtn;
	GLButton* facetDetailsBtn;
	GLButton* facetCoordBtn;
	GLButton* facetAdvParamsBtn; // <<Adv, used by Molflow only
	GLTitledPanel* facetPanel;
	GLList* facetList;
	GLLabel* facetSideLabel;
	GLTitledPanel* togglePanel;
	GLCombo* facetSideType;
	GLLabel* facetTLabel;
	GLTextField* facetOpacity;
	GLLabel* facetAreaLabel;
	GLTextField* facetAreaText;

	GLToggle* autoFrameMoveToggle;
	GLButton* forceFrameMoveButton;

	GLLabel* hitLabel;
	GLLabel* desLabel;
	GLLabel* leakLabel;
	GLLabel* sTimeLabel;

	GLTitledPanel* shortcutPanel;
	GLTitledPanel* simuPanel;

	GLMenu* structMenu;
	GLMenu* viewsMenu;
	GLMenu* selectionsMenu;
	GLMenu* memorizeSelectionsMenu;
	GLMenu* memorizeViewsMenu;
	GLMenu* clearSelectionsMenu;
	GLMenu* clearViewsMenu;

	// Views
	void SelectView(int v);
	void AddView(const CameraView& v);
	void AddView();
	void ClearViewMenus() const;
	void ClearAllViews();
	void OverWriteView(int idOvr);
	void ClearView(int idClr);
	void RebuildViewMenus();

	// Selections
	void SelectSelection(size_t v);
	void AddSelection(const SelectionGroup& s);
	void AddSelection(const std::string& selectionName);
	void ClearSelectionMenus() const;
	void ClearAllSelections();
	void OverWriteSelection(size_t idOvr);
	void ClearSelection(size_t idClr);
	void RebuildSelectionMenus();

	void UpdateFacetlistSelected();

	void CreateOfTwoFacets(Clipper2Lib::ClipType type, int reverseOrder = 0);
	//void UpdateMeasurements();
	void DropEvent(char* dropped_file);
	bool AskToSave();
	bool AskToReset(Worker* work = nullptr);
	void AddStruct();
	void DeleteStruct();

	void SaveFileAs();
	bool AutoSave(bool crashSave = false);
	void ResetAutoSaveTimer();
	void CheckForRecovery();
	void SetDefaultViews();

	std::vector<CameraView>   views;
	int     curViewer;
	int     modeSolo;

	std::vector<SelectionGroup> selections;
	size_t idSelection; //Allows "select next" / "select previous" commands

	//Shared windows (implementation might be Molflow/Synrad-specific)
	FacetDetails* facetDetails = nullptr;
	Viewer3DSettings* viewer3DSettings = nullptr;
	TextureScaling* textureScaling = nullptr;
	GlobalSettings* globalSettings = nullptr;
	ProfilePlotter* profilePlotter = nullptr;
	TexturePlotter* texturePlotter = nullptr;
	CollapseSettings* collapseSettings = nullptr;
	HistogramSettings* histogramSettings = nullptr;
	HistogramPlotter* histogramPlotter = nullptr;
	MoveVertex* moveVertex = nullptr;
	ScaleFacet* scaleFacet = nullptr;
	ScaleVertex* scaleVertex = nullptr;
	SelectDialog* selectDialog = nullptr;
	SelectTextureType* selectTextureType = nullptr;
	SelectFacetByResult* selectFacetByResult = nullptr;
	ExtrudeFacet* extrudeFacet = nullptr;
	MoveFacet* moveFacet = nullptr;
	ParticleLogger* particleLogger = nullptr;
	MirrorFacet* mirrorFacet = nullptr;
	MirrorVertex* mirrorVertex = nullptr;
	CreateShape* createShape = nullptr;
	SplitFacet* splitFacet = nullptr;
	BuildIntersection* buildIntersection = nullptr;
	RotateFacet* rotateFacet = nullptr;
	RotateVertex* rotateVertex = nullptr;
	AlignFacet* alignFacet = nullptr;
	AddVertex* addVertex = nullptr;
	FacetCoordinates* facetCoordinates = nullptr;
	VertexCoordinates* vertexCoordinates = nullptr;
	SmartSelection* smartSelection = nullptr;
	FormulaEditor* formulaEditor = nullptr;
	ConvergencePlotter* convergencePlotter = nullptr;
	CrossSection* crossSectionWindow = nullptr;

	UpdateCheckDialog* updateCheckDialog = nullptr;
	UpdateFoundDialog* updateFoundDialog = nullptr;
	UpdateLogWindow* updateLogWindow = nullptr;
	ManualUpdateCheckDialog* manualUpdate = nullptr;

	void LoadSelection(const char* fName = nullptr);
	void SaveSelection();
	void ExportSelection();
	void UpdateModelParams();
	void UpdateViewerFlags();
	void ResetSimulation(bool askConfirm = true);
	void UpdateStructMenu();
	void UpdateTitle();

	void AnimateViewerChange(int next);
	void UpdateViewerPanel();

	virtual void SelectViewer(int s);

	void Place3DViewer();
	void UpdateViewers();
	void SetFacetSearchPrg(bool visible, const char* text);

	void DisplayCollapseDialog();
	void RenumberSelections(const std::vector<int>& newRefs);
	int  Resize(size_t width, size_t height, bool forceWindowed) override;

	// Formula management
	static bool OffsetFormula(std::string& expression, int offset, int filter = -1, std::vector<int>* newRefs = nullptr);
	void RenumberFormulas(std::vector<int>* newRefs) const;
	void ClearFormulas() const;

	void ExportTextures(int grouping, int mode);

	// Recent files
	std::vector<std::string> recentsList; //Last in the vector is latest file (top in menu)
	void AddRecent(const std::string& fileName);
	void RemoveRecent(const std::string& fileName);
	void UpdateRecentMenu();

	bool needsMesh;    //At least one viewer displays mesh
	bool needsTexture; //At least one viewer displays textures
	bool needsDirection; //At least one viewer displays direction vectors
	void CheckNeedsTexture();
	void DoEvents(bool forced = false); //Used to catch button presses (check if an abort button was pressed during an operation)

protected:
	void OneTimeSceneInit_shared_pre();
	void OneTimeSceneInit_shared_post();
	int RestoreDeviceObjects_shared();
	int InvalidateDeviceObjects_shared();
	bool ProcessMessage_shared(GLComponent* src, int message);
	void  BeforeExit() override;
};