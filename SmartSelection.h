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

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"

#include "Geometry.h"
#include "Worker.h"

#pragma once

class SmartSelection : public GLWindow {

public:

  // Construction
  SmartSelection(Geometry *g,Worker *w);
  BOOL IsSmartSelection();
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

  BOOL isRunning;
};

