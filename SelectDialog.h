/*
  File:        GLSelectDialog.h
  Description: Select facet by number dialog

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef _GLSELECTDIALOGH_
#define _GLSELECTDIALOGH_

#include "GLApp/GLWindow.h"

class GLTextField;
class GLButton;
class GLLabel;

class Geometry;
class Worker;

// Buttons
#ifndef GLDLG_CANCEL
#define GLDLG_CANCEL		0x0002
#endif

#ifndef GLDLG_SELECT
#define GLDLG_SELECT		0x0004
#endif

#ifndef GLDLG_SELECT_ADD
#define GLDLG_SELECT_ADD	0x0008
#endif

#ifndef GLDLG_SELECT_REM
#define GLDLG_SELECT_REM	0x0016
#endif

class SelectDialog : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  SelectDialog(Worker *w);
  int  rCode;
  void ProcessMessage(GLComponent *src,int message);
private:
 

  Geometry     *geom;
  Worker	   *work;
  GLTextField *numText;
};

#endif /* _GLSELECTDIALOGH_ */
