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

#ifndef MOLFLOW_PROJ_IMGUISIDEBAR_H
#define MOLFLOW_PROJ_IMGUISIDEBAR_H

class Geometry;
#if defined(MOLFLOW)
class MolFlow;
void ShowAppSidebar(bool *p_open, MolFlow *mApp, Geometry *guiGeom, bool *show_global, bool *newViewer);
#else
class SynRad;
void ShowAppSidebar(bool *p_open, SynRad *mApp, Geometry *guiGeom, bool *show_global, bool *newViewer);
#endif

#endif //MOLFLOW_PROJ_IMGUISIDEBAR_H
