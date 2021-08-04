//
// Created by pbahr on 8/2/21.
//

#ifndef MOLFLOW_PROJ_IMGUIAABB_H
#define MOLFLOW_PROJ_IMGUIAABB_H

#include "transfer_function_widget.h"

class MolFlow;
class TransferFunctionWidget;

class ImguiAABBVisu {
    TransferFunctionWidget tfn_widget;
public:
    ImguiAABBVisu();
    void ShowAABB(MolFlow *mApp, bool *show_aabb, bool &redrawAabb, bool &rebuildAabb);
};

#endif //MOLFLOW_PROJ_IMGUIAABB_H
