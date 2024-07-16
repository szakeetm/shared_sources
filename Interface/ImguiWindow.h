#pragma once
#include "imgui.h"
#include <GLApp/GLApp.h>
#include "AppUpdater.h"
#include "ImguiPopup.h"
#include "ImguiSmartSelection.h"
#include "ImguiSelectDialog.h"
#include "ImguiSelectTextureType.h"
#include "Helper/GLProgress_ImGui.h"
#include "ImguiShortcutManager.h"
#include "ImguiSidebar.h"
#include "ImguiFacetMove.h"
#include "ImguiGlobalSettings.h"
#include "ImguiSelectFacetByResult.h"
#include "ImguiFormulaEditor.h"
#include "ImguiConvergencePlotter.h"
#include "ImguiTexturePlotter.h"
#include "ImguiProfilePlotter.h"
#include "ImguiHistogramPlotter.h"
#include "ImguiParticleLogger.h"
#include "ImguiMovingParts.h"
#include "ImguiMeasureForce.h"
#include "ImguiFacetCoordinates.h"
#include "ImguiFacetScale.h"
#include "ImguiFacetMirrorProject.h"
#include "ImguiFacetRotate.h"
#include "ImguiFacetAlign.h"
#include "ImguiFacetExtrude.h"
#include "ImguiFacetSplit.h"
#include "ImguiCreateShape.h"
#include "ImguiBuildIntersection.h"
#include "ImguiCollapseSettings.h"
#include "ImguiOutgassingMap.h"
#include "ImguiVertexCoordinates.h"
#include "ImguiVertexMove.h"
#include "ImguiViewer3DSettings.h"
#include "ImguiAdvFacetParams.h"
#include "ImguiFacetDetails.h"

#include "ImguiMenu.h"

#include "ImguiGeometryViewer.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "../../src/Interface/ImguiTextureScaling.h"
#else
#include "../../src/SynRad.h"
#endif

#ifdef ENABLE_IMGUI_TESTS
#include "../imguiTesting.h"
#endif

/*! Window manager for the Imgui GUI, right now it is rendered above the SDL2 GUI */
class ImguiWindow {
public:
    bool forceDrawNextFrame = false;
    bool skipImGuiEvents = false;
    unsigned int textCursorPos = 0;
    explicit ImguiWindow(GLApplication* app) {this->app = app;};
    void init();
    void destruct();
    //void render();
    void renderSingle(); //!< Main function, calling a frame rendering cycle handling all other Imgui windows

    void Refresh();
    void Reset();
    void Clear();
    void LoadProfileFromFile(const std::unique_ptr<MolflowInterfaceSettings>& interfaceSettings);

    GLApplication* app;

#ifdef ENABLE_IMGUI_TESTS
    ImTest testEngine;
#endif

    bool ToggleMainHub();
    bool ToggleMainMenu();
    bool ToggleDemoWindow();
    // Window states (visible or not)
    bool show_main_hub{false}; //!< Hub managing all other windows
    bool show_app_main_menu_bar{false}; //!< Window main menu bar at the top
    bool show_demo_window{false}; //!< Debug only: ImGui Demo Window to test all ImGui functionalities
    bool show_perfo{false}; //!< Plot showing history of simulation performance
    bool show_window_license{false};

    void ShowWindowLicense();
    ImIOWrappers::ImPopup popup;
    ImIOWrappers::ImInputPopup input;
    ImProgress progress;
    ImSmartSelection smartSelect;
    ImSelectDialog selByNum;
    ImSelectTextureType selByTex;
    ImFacetMove facetMov;
    ShortcutManager shortcutMan;
    ImSidebar sideBar;
    ImAdvFacetParams advFacPar;
    ImViewerSettings viewSet;
    ImGlobalSettings globalSet;
    ImSelectFacetByResult selFacetByResult;
    ImFormulaEditor formulaEdit;
    ImConvergencePlotter convPlot;
    ImTexturePlotter textPlot;
    ImProfilePlotter profPlot;
    ImHistogramPlotter histPlot;
#if defined(MOLFLOW)
    ImTextureScaling textScale;
#endif
    ImParticleLogger partLog;
    ImMovingParts movPart;
    ImMeasureForce measForce;
    ImFacetCoordinates facCoord;
    ImFacetScale facScale;
    ImFacetMirrorProject mirrProjFacet;
    ImFacetRotate rotFacet;
    ImFacetAlign alignFacet;
    ImFacetExtrude extrudeFacet;
    ImFacetSplit splitFac;
    ImCreateShape createShape;
    ImBuildIntersect buildIntersect;
    ImCollapse collapseSettings;
    ImOutgassingMap outgassingMap;
    ImVertexCoordinates vertCoord;
    ImVertexMove vertMov;

    ImFacetDetails facDet;
    ImExplodeFacet expFac;

    ImGeoViewer geoView;

    std::string ctrlText = "CTRL";
    ImGuiKey modifier = ImGuiKey_LeftCtrl;
protected:
    bool didIinit = false;
    ImGuiConfigFlags storedConfigFlags = 0;

    double start_time = 0.f; // to keep track how long the ImGui GUI is running
};