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

#include "GLApp/GLWindow.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "Worker.h"

#ifndef _GLSELECTDIALOGH_
#define _GLSELECTDIALOGH_

// Buttons
#define GLDLG_CANCEL_SEL    0x0001
#define GLDLG_SELECT		0x0002
#define GLDLG_SELECT_ADD	0x0004
#define GLDLG_SELECT_REM    0x0008

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
