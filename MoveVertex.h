/*
  File:        MoveVertex.h
  Description: Move vertex by offset dialog
*/

#ifndef _MOVEVERTEXH_
#define _MOVEVERTEXH_

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class Geometry;
class Worker;

class MoveVertex : public GLWindow {

public:

  // Construction
  MoveVertex(Geometry *geom,Worker *work);

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

  int nbVertexS;

};

#endif /* _MOVEVERTEXH_ */
