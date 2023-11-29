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
    std::vector<size_t> facetIds; // List of facets
};

//Don't change enum order! Must be backward compatible with int (file save, combobox index)
enum ProjectionMode : int {
    Perspective,
    Orthographic
};

enum CameraPlaneMode : int {
    CamNone,
    CamTop,
    CamSide,
    CamFront
};

enum VolumeRenderMode : int {
    FrontAndBack,
    FrontOnly,
    BackOnly
};


struct Plane {
    //XY by default
    Plane() = default;
    Plane(double a_, double b_, double c_, double d_) : a(a_), b(b_), c(c_), d(d_) {}
    double a = 0.0;
    double b = 0.0;
    double c = 1.0;
    double d = 0.0;
};
// Definition of a view. Note: all basis are left handed

struct CameraView{

    std::string name;    // View name

    ProjectionMode      projMode = ProjectionMode::Orthographic;   // Projection type
    double   camAngleOx=0.0; // Spheric coordinates. Right-hand rotation (in left-hand coord.sys)
    double   camAngleOy=0.0; // Spheric coordinates Left-hand rotation (in left-hand coord.sys)
    double   camAngleOz=0.0; // Rotation around third axis

    double   camDist=100.0;    // Camera distance (or zoom in orthographic)

    double   lightAngleOx=0.0; //Light direction
    double   lightAngleOy=0.0; //Light direction

    Vector3d camOffset;  // Camera target offset
    CameraPlaneMode      performXY= CameraPlaneMode::CamNone;  // Draw x,y,z coordinates when aligned with axis and orthographic

    double   vLeft=0.0;      // Viewport in 2D proj space (used for orthographic autoscaling)
    double   vRight = 0.0;     // Viewport in 2D proj space (used for orthographic autoscaling)
    double   vTop = 0.0;       // Viewport in 2D proj space (used for orthographic autoscaling)
    double   vBottom = 0.0;    // Viewport in 2D proj space (used for orthographic autoscaling)

    double cutFactor = 0.0;

    bool enableClipping = false; // Cross section through clip plane
    Plane clipPlane;
};

struct UserFormula {
    std::string name, expression;
};

struct PlotterSetting {
    bool hasData = false; //If has actual data, set to true
    bool logYscale=false;
    std::vector<int> viewIds;
};

struct FacetInterfaceSetting { //Extra information that can't be stored in a SimulationFacet class. Used for loading/writing facets
    bool textureVisible=true;
    bool volumeVisible=true;
};

struct InterfaceSettings { //extra information (not part of SimulationModel) in XML file used by XmlLoader and XmlWriter, then passed to/from GUI
    std::vector<SelectionGroup> selections;
    std::vector<CameraView> views;
    std::vector<UserFormula> userFormulas;
    std::vector<FacetInterfaceSetting> facetSettings;
    PlotterSetting profilePlotterSettings;
    PlotterSetting convergencePlotterSettings;
    bool  texAutoScale = true;  // Autoscale flag
    bool  texColormap = true;   // Colormap flag
    bool  texLogScale = true;   // Texture im log scale
    int textureMode = 0; //Physics mode
};
