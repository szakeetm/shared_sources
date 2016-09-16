/*
  File:        LoadStatus.h
  Description: Subprocess load status
  Program:     SynRad

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "Worker.h"
#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp\GLList.h"

#ifndef _LOADSTATUSH_
#define _LOADSTATUSH_

class LoadStatus : public GLWindow {

public:

  // Construction
  LoadStatus(Worker *w);
  void SMPUpdate();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  GLList      *processList;
  GLLabel     *memInfoLabel;
  GLButton    *cancelButton;
  Worker      *worker;
  
};

#endif /* _LOADSTATUSH_ */
