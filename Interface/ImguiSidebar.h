//
// Created by Pascal Baehr on 24.04.22.
//

#ifndef MOLFLOW_PROJ_IMGUISIDEBAR_H
#define MOLFLOW_PROJ_IMGUISIDEBAR_H

#if defined(MOLFLOW)
class MolFlow;
#else
class SynRad;
#endif
class Geometry;

void ShowAppSidebar(bool *p_open, MolFlow *mApp, Geometry *geom, bool *show_global, bool *newViewer);
#endif //MOLFLOW_PROJ_IMGUISIDEBAR_H
