#pragma once
#include "GLApp\GLWindow.h"

class Worker;
class Geometry;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLButton;

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

class SelectTextureType : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  SelectTextureType(Worker *w);
  int  rCode;
  void ProcessMessage(GLComponent *src,int message);
private:
  Geometry     *geom;
  Worker	   *work;
  GLButton
	  *selectButton,
	  *addSelectButton,
	  *remSelectButton;
  GLTextField
	  *ratioText,
	  *ratioMinText,
	  *ratioMaxText;
  GLToggle
	  *ratioToggle,
	  *ratioMinMaxToggle,
	  *desorbToggle,
	  *absorbToggle,
	  *reflectToggle,
	  *transparentToggle,
	  *directionToggle;
  GLTitledPanel
	  *resolutionpanel,
	  *textureTypePanel;
};
