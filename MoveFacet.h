/*
  File:        MoveFacet.h
  Description: Move facet by offset dialog
*/

#ifndef _MOVEFACETH_
#define _MOVEFACETH_

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class Geometry;
class Worker;

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
