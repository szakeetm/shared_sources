/*
  File:        ScaleVertex.h
  Description: Mirror facet to plane dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _ScaleVertexH_
#define _ScaleVertexH_

class ScaleVertex : public GLWindow {

public:
  // Construction
  ScaleVertex(Geometry *geom,Worker *work);
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLButton     *scaleButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLTextField *xText;
  GLTextField *yText;
  GLTextField *zText;
  GLTextField *vertexNumber;
  GLTextField *facetNumber;
  GLTextField *factorNumber;

  int nbFacetS,invariantMode;

  Geometry     *geom;
  Worker	   *work;


};

#endif /* _ScaleVertexH_ */
