//
// Created by pascal on 2/26/21.
//

#ifndef MOLFLOW_PROJ_IMGUIWINDOW_H
#define MOLFLOW_PROJ_IMGUIWINDOW_H


#include <GLApp/GLApp.h>
#include <imgui/imgui.h>
#include "AppUpdater.h"
#include "../../src/MolFlow.h"

class ImguiWindow {
public:
    explicit ImguiWindow(GLApplication* app) {this->app = app;};
    void init();
    void destruct();
    //void render();
    void renderSingle();

    GLApplication* app;

    bool ToggleMainMenu(){
        show_app_main_menu_bar = !show_app_main_menu_bar;
        return show_app_main_menu_bar;
    }
    bool ToggleSimStatus(){
        show_app_sim_status = !show_app_sim_status;
        return show_app_sim_status;
    }
    bool ToggleDemoWindow(){
        show_demo_window = !show_demo_window;
        return show_demo_window;
    }
    bool ToggleGlobalSettings(){
        show_global_settings = !show_global_settings;
        return show_global_settings;
    }
    bool ToggleAABBWindow(){
        show_aabb = !show_aabb;
        return show_aabb;
    }
protected:
    // Our state
    bool show_app_main_menu_bar{false};
    bool show_app_sim_status{false};
    bool show_demo_window{false};
    bool show_global_settings{true};
    bool show_aabb{true};
    bool show_perfo{true};

    ImVec4 clear_color;

    static void restartProc(int nbProc, MolFlow *mApp);

    //static void ProcessControlTable(MolFlow *mApp);

    //static bool InputRightSide(const char *desc, double *const &val, const char *format);
};


#endif //MOLFLOW_PROJ_IMGUIWINDOW_H
