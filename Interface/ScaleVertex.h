/*
  File:        ScaleVertex.h
  Description: Mirror facet to plane dialog
*/

#ifndef _ScaleVertexH_
#define _ScaleVertexH_

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class InterfaceGeometry;
class Worker;

class ScaleVertex : public GLWindow {

public:
  // Construction
  ScaleVertex(InterfaceGeometry *interfGeom,Worker *work);
  void ProcessMessage(GLComponent *src,int message) override;

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLTitledPanel *sPanel;
  GLButton     *scaleButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLButton    *getSelVertexButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLTextField *xText;
  GLTextField *yText;
  GLTextField *zText;
  GLTextField *vertexNumber;
  GLTextField *factorNumber;
  GLTextField *OnePerFactor;
  GLTextField *factorNumberX;
  GLTextField *factorNumberY;
  GLTextField *factorNumberZ;
  GLToggle    *uniform;
  GLToggle    *distort;

  int nbFacetS, invariantMode, scaleMode;

  InterfaceGeometry     *interfGeom;
  Worker	   *work;
};

#endif /* _ScaleVertexH_ */
