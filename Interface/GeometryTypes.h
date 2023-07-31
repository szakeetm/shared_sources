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

#define SYNVERSION   12
//12: added newReflectionModel, lowFluxMode, lowFluxCutoff (previously app settings)

#define PARAMVERSION 5
//4: added structureId
//5: added globalComponents, startS

//Structs used by file loaders/writers

typedef struct {
    std::string    name;       // Selection name
    std::vector<size_t> facetIds; // List of facets
} SelectionGroup;

// Definition of a view. Note: all basis are left handed

typedef struct {

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

} AVIEW;

struct UserFormula {
    std::string name, expression;
};

struct PlotterSetting {
    bool logYscale=false;
    std::vector<int> viewIds;
};

struct FacetViewSetting {
    bool textureVisible=true;
    bool volumeVisible=true;
};

struct UserSettings {
    //user settings such as selections, facet view settings, parameters and moments, that must be persistent even in CLI
    std::vector<SelectionGroup> selections;
    std::vector<AVIEW> views;
    std::vector<UserFormula> userFormulas;
    std::vector<FacetViewSetting> facetViewSettings;
    std::vector<UserMoment> userMoments;
    std::unique_ptr<PlotterSetting> profilePlotterSettings; //nullptr if couldn't be loaded
    std::unique_ptr<PlotterSetting> convergencePlotterSettings; //nullptr if couldn't be loaded
};
