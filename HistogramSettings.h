/*
Program:     MolFlow+
Description: Monte Carlo simulator for ultra-high vacuum
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
*/
#pragma once

#include "GLApp/GLWindow.h"
#include <vector>

class GLWindow;
class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;

class Geometry;
class Worker;

class HistogramSettings : public GLWindow {

public:

  // Construction
  HistogramSettings();
  void Reposition();

  // Component methods
  void SetGeometry(Geometry *s,Worker *w);
  void Refresh(const std::vector<size_t>& selectedFacetIds);
  bool Apply();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  //GLButton    *applyButton;
  GLTextField *distanceLimit;
  GLTextField *hitLimit;
  GLTextField *hitBinSize;
  GLToggle *recordToggle;
  GLLabel  *memoryEstimateLabel;

  size_t nbFacetSelected;

};