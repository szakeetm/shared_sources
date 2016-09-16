/*
  File:        ScaleFacet.h
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

#ifndef _ScaleFacetH_
#define _ScaleFacetH_

class ScaleFacet : public GLWindow {

public:
  // Construction
  ScaleFacet(Geometry *geom,Worker *work);
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLTitledPanel *sPanel;
  GLButton     *scaleButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLTextField *xText;
  GLTextField *yText;
  GLTextField *zText;
  GLTextField *facetNumber;
  GLTextField *factorNumber;
  GLTextField *factorNumberX;
  GLTextField *factorNumberY;
  GLTextField *factorNumberZ;
  GLToggle    *uniform;
  GLToggle    *distort;

  int nbFacetS,invariantMode,scaleMode;

  Geometry     *geom;
  Worker	   *work;


};

#endif /* _ScaleFacetH_ */
