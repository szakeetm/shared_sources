
#pragma once

#include "GLApp/GLWindow.h"
class GLWindow;
class GLButton;
class GLTextField;
class GLLabel;

//#include "Geometry_shared.h"
//#include "Worker.h"
class InterfaceGeometry;
class Worker;

class AddVertex : public GLWindow {

public:

  // Construction
  AddVertex(InterfaceGeometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

  GLButton    *addButton;
  GLButton    *facetCenterButton;
  GLButton    *facetUButton;
  GLButton    *facetVButton;
  GLButton    *facetNormalButton;
  GLTextField *x;
  GLTextField *y;
  GLTextField *z;

};