//
// Created by pascal on 2/26/21.
//

#ifndef MOLFLOW_PROJ_IMGUIWINDOW_H
#define MOLFLOW_PROJ_IMGUIWINDOW_H


#include <GLApp/GLApp.h>

class ImguiWindow {
public:
    explicit ImguiWindow(GLApplication* app) {this->app = app;};
    void init();
    void destruct();
    void render();
    void renderSingle();

    GLApplication* app;
};


#endif //MOLFLOW_PROJ_IMGUIWINDOW_H
