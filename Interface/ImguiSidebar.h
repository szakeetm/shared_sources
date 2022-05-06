//
// Created by Pascal Baehr on 24.04.22.
//

#ifndef MOLFLOW_PROJ_IMGUISIDEBAR_H
#define MOLFLOW_PROJ_IMGUISIDEBAR_H

class Geometry;
#if defined(MOLFLOW)
class MolFlow;
void ShowAppSidebar(bool *p_open, MolFlow *mApp, Geometry *geom, bool *show_global, bool *newViewer);
#else
class SynRad;
void ShowAppSidebar(bool *p_open, SynRad *mApp, Geometry *geom, bool *show_global, bool *newViewer);
#endif

#endif //MOLFLOW_PROJ_IMGUISIDEBAR_H
