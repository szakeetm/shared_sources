#pragma once
#include "GLApp/GLWindow.h"

class Worker;
class InterfaceGeometry;
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

class SelectFacetByResult : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
	SelectFacetByResult(Worker *w);
  void ProcessMessage(GLComponent *src,int message) override;
private:
  InterfaceGeometry     *interfGeom;
  Worker	   *work;
  GLButton
	  *selectButton,
	  *addSelectButton,
	  *remSelectButton;
  GLTextField *hitsLessThanText,*hitsMoreThanText;
	 GLTextField *absLessThanText,*absMoreThanText;
	 #ifdef MOLFLOW
	  GLTextField *desLessThanText,*desMoreThanText;
	  #endif
	  #ifdef SYNRAD
	   GLTextField *fluxLessThanText,*fluxMoreThanText;
	   GLTextField *powerLessThanText,*powerMoreThanText;	  
	   #endif
};
