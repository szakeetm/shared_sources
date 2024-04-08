#pragma once

#include "GLApp/GLWindow.h"

class GLTextField;
class GLButton;
class GLLabel;

class InterfaceGeometry;
class Worker;

class SelectDialog : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  SelectDialog(InterfaceGeometry *g);
  int  rCode;
  void ProcessMessage(GLComponent *src,int message) override;
private:
 
	GLButton * selButton, *addButton, *remButton;
  InterfaceGeometry     *interfGeom;
  Worker	   *work;
  GLTextField *numText;
};
