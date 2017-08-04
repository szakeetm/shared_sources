/*
  File:        RecoveryDialog.h
  Description: Autosave file recovery dialog

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef _GLRECOVERYDIALOGH_
#define _GLRECOVERYDIALGOH_

//#include <SDL_opengl.h>
#include "GLApp/GLWindow.h"

// Buttons
#define GLDLG_LOAD		0x0001
#define GLDLG_SKIP		0x0002
#define GLDLG_DELETE	0x0008
#define GLDLG_CANCEL_R    0x0016

// Icons
#define GLDLG_ICONNONE    0
#define GLDLG_ICONERROR   1
#define GLDLG_ICONWARNING 2
#define GLDLG_ICONINFO    3

class RecoveryDialog : private GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  static int Display(const char *message,char *title=NULL,int mode=GLDLG_LOAD|GLDLG_SKIP|GLDLG_DELETE,int icon=GLDLG_ICONINFO);
  int  rCode;

private:
	RecoveryDialog(const char *message, char *title, int mode, int icon);
  void ProcessMessage(GLComponent *src,int message);

};

#endif
