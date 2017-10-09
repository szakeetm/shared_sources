#pragma once

#include "GLApp/GLWindow.h"
class GLWindow;
class GLButton;
class GLTextField;
class GLLabel;

//#include "Geometry_shared.h"
//#include "Worker.h"
class Geometry;
class Worker;

class AddVertex : public GLWindow {

public:

  // Construction
  AddVertex(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
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