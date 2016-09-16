/*
  File:        MirrorFacet.h
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

#ifndef _MIRRORFACETH_
#define _MIRRORFACETH_

class MirrorFacet : public GLWindow {

public:
  // Construction
  MirrorFacet(Geometry *geom,Worker *work);
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
  GLTextField *aText;
  GLTextField *bText;
  GLTextField *cText;
  GLTextField *dText;
  GLTextField *facetNumber;
  GLLabel		*aLabel;
  GLLabel		*bLabel;
  GLLabel		*cLabel;
  GLLabel		*dLabel;

  int nbFacetS;

  Geometry     *geom;
  Worker	   *work;


};

#endif /* _MirrorFacetH_ */
