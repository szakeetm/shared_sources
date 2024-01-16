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
#include <GLApp/GLApp.h>
#include <imgui/imgui.h>
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
#include "ImguiTextureScaling.h"
#include "ImguiParticleLogger.h"
#include "ImguiMovingParts.h"
#include "ImguiMeasureForce.h"
#include "ImguiFacetCoordinates.h"
#include "ImguiFacetScale.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#else
#include "../../src/SynRad.h"
#endif

/*! Window manager for the Imgui GUI, right now it is rendered above the SDL2 GUI */
class ImguiWindow {
public:
    explicit ImguiWindow(GLApplication* app) {this->app = app;};
    void init();
    static void destruct();
    //void render();
    void renderSingle(); //!< Main function, calling a frame rendering cycle handling all other Imgui windows

    void Refresh();
    void Reset();
    void LoadProfileFromFile(const std::unique_ptr<MolflowInterfaceSettings>& interfaceSettings);

    GLApplication* app;

    bool ToggleMainHub();
    bool ToggleMainMenu();
    bool ToggleSimSidebar();
    bool ToggleDemoWindow();
    // Window states (visible or not)
    bool show_main_hub{false}; //!< Hub managing all other windows
    bool show_app_main_menu_bar{false}; //!< Window main menu bar at the top
    bool show_app_sidebar{false}; //!< Simulation sidebar for various 3d viewer, facet and simulation settings
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
    ImGuiSidebar sideBar;
    ImGlobalSettings globalSet;
    ImSelectFacetByResult selFacetByResult;
    ImFormulaEditor formulaEdit;
    ImConvergencePlotter convPlot;
    ImTexturePlotter textPlot;
    ImProfilePlotter profPlot;
    ImHistogramPlotter histPlot;
    ImTextureScaling textScale;
    ImParticleLogger partLog;
    ImMovingParts movPart;
    ImMeasureForce measForce;
    ImFacetCoordinates facCoord;
    ImFacetScale facScale;
protected:
    bool didIinit = false;
    ImGuiConfigFlags storedConfigFlags;

    double start_time; // to keep track how long the ImGui GUI is running
};