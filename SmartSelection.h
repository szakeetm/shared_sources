/*
  File:        SmartSelection.h
  Description: Smart selection dialog
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY
  Copyright:   E.S.R.F / CERN

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

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;

class Geometry;
class Worker;

class SmartSelection : public GLWindow {

public:

  // Construction
  SmartSelection(Geometry *g,Worker *w);
  bool IsSmartSelection();
  double GetMaxAngle();
  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  GLButton    *analyzeButton;
  GLLabel     *resultLabel;
  GLTextField *angleThreshold;
  GLToggle    *enableToggle;

  bool isRunning;
};

