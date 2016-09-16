/*
  File:        MoveFacet.h
  Description: Move facet by offset dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _MOVEFACETH_
#define _MOVEFACETH_

class MoveFacet : public GLWindow {

public:

  // Construction
  MoveFacet(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  GLButton    *moveButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLLabel     *l1;
  GLLabel     *l2;
  GLLabel     *l3;
  GLTextField *xOffset;
  GLTextField *yOffset;
  GLTextField *zOffset;

  int nbFacetS;


};

#endif /* _MoveFacetH_ */
