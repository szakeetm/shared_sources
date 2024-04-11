
#pragma once


#include "GLApp/GLWindow.h"
class GLWindow;
class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;

//#include "Geometry_shared.h"
//#include "Worker.h"
class InterfaceGeometry;
class Worker;

class CollapseSettings : public GLWindow {

public:

  // Construction
  CollapseSettings();

  // Component methods
  void SetGeometry(InterfaceGeometry *s,Worker *w);

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

  GLButton    *goButton;
  GLButton    *goSelectedButton;
  GLButton    *cancelButton;
  GLLabel     *resultLabel;
  GLTextField *vThreshold;
  GLTextField *pThreshold;
  GLTextField *lThreshold;
  GLTextField* maxVertexTextbox;
  GLToggle *l1;
  GLToggle *l2;
  GLToggle *l3;
  GLToggle* maxVertexToggle;

  bool isRunning;

  size_t nbVertexS;
  size_t nbFacetS;
  size_t nbFacetSS;

};