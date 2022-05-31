//
// Created by pascal on 2/26/21.
//

#ifndef MOLFLOW_PROJ_IMGUIWINDOW_H
#define MOLFLOW_PROJ_IMGUIWINDOW_H


#include <GLApp/GLApp.h>
#include <imgui/imgui.h>
#include "AppUpdater.h"

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

    GLApplication* app;

    bool ToggleMainHub();
    bool ToggleMainMenu();
    bool ToggleSimSidebar();
    bool ToggleDemoWindow();
    bool ToggleGlobalSettings();
    bool ToggleAABBWindow(){
        show_aabb = !show_aabb;
        return show_aabb;
    }
protected:
    // Window states (visible or not)
    bool show_main_hub{false}; //!< Hub managing all other windows
    bool show_app_main_menu_bar{false}; //!< Window main menu bar at the top
    bool show_app_sidebar{false}; //!< Simulation sidebar for various 3d viewer, facet and simulation settings
    bool show_demo_window{false}; //!< Debug only: ImGui Demo Window to test all ImGui functionalities
    bool show_global_settings{false}; //!< Global Settings window
    bool show_perfo{false}; //!< Plot showing history of simulation performance
    bool show_aabb{false};
    bool show_select{false};

    double start_time; // to keep track how long the ImGui GUI is running
    static void restartProc(int nbProc, MolFlow *mApp);
};


#endif //MOLFLOW_PROJ_IMGUIWINDOW_H
