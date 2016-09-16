/*
  File:        MoveVertex.h
  Description: Move vertex by offset dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _MOVEVERTEXH_
#define _MOVEVERTEXH_

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
