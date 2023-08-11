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

#include <string>
#include <vector>
#include "Vector.h"

#define SYNVERSION   12
//12: added newReflectionModel, lowFluxMode, lowFluxCutoff (previously app settings)

#define PARAMVERSION 5
//4: added structureId
//5: added globalComponents, startS

//Structs used by file loaders/writers

struct SelectionGroup{
    std::string    name;       // Selection name
    std::vector<int> facetIds; // List of facets
};

// Definition of a view. Note: all basis are left handed

struct CameraView{

    std::string name;    // View name

    int      projMode;   // Projection type
    double   camAngleOx; // Spheric coordinates. Right-hand rotation (in left-hand coord.sys)
    double   camAngleOy; // Spheric coordinates Left-hand rotation (in left-hand c.sys)

    double   camAngleOz; // Rotation around third axis

    double   camDist;    // Camera distance (or zoom in orthographic)

    double   lightAngleOx; //Light direction
    double   lightAngleOy; //Light direction

    Vector3d camOffset;  // Camera target offset
    int      performXY;  // Draw x,y,z coordinates when aligned with axis and orthographic

    double   vLeft;      // Viewport in 2D proj space (used for orthographic autoscaling)
    double   vRight;     // Viewport in 2D proj space (used for orthographic autoscaling)
    double   vTop;       // Viewport in 2D proj space (used for orthographic autoscaling)
    double   vBottom;    // Viewport in 2D proj space (used for orthographic autoscaling)

};

struct UserFormula {
    std::string name, expression;
};

struct PlotterSetting {
    bool hasData = false; //If has actual data, set to true
    bool logYscale=false;
    std::vector<int> viewIds;
};

struct FacetViewSetting {
    bool textureVisible=true;
    bool volumeVisible=true;
};

struct UserSettings { //extra information (not part of SimulationModel) in XML file used by XmlLoader and XmlWriter, then passed to/from GUI
    std::vector<SelectionGroup> selections;
    std::vector<CameraView> views;
    std::vector<UserFormula> userFormulas;
    std::vector<FacetViewSetting> facetViewSettings;
    PlotterSetting profilePlotterSettings;
    PlotterSetting convergencePlotterSettings;
};
