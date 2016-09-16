/*
  File:        RotateFacet.h
  Description: Rotate facet around axis dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _ROTATEFACETH_
#define _ROTATEFACETH_

class RotateFacet : public GLWindow {

public:
  // Construction
  RotateFacet(Geometry *geom,Worker *work);
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLButton     *moveButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLToggle     *l4;
  GLToggle     *l5;
  GLToggle     *l6;
  GLLabel     *lNum;
  GLToggle     *l7;
  GLToggle     *l8;
  GLTextField *aText;
  GLTextField *bText;
  GLTextField *cText;
  GLTextField *uText;
  GLTextField *vText;
  GLTextField *wText;
  GLTextField *facetNumber;
  GLTextField *degText;
  GLLabel		*aLabel;
  GLLabel		*bLabel;
  GLLabel		*cLabel;
  GLLabel		*uLabel;
  GLLabel		*vLabel;
  GLLabel		*wLabel;
  GLLabel		*degLabel;

  int nbFacetS;

  Geometry     *geom;
  Worker	   *work;


};

#endif /* _RotateFacetH_ */
