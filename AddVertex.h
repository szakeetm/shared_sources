/*
  File:        AddVertex.h
  Description: Add new vertex dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _ADDVERTEXH_
#define _ADDVERTEXH_

class AddVertex : public GLWindow {

public:

  // Construction
  AddVertex(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  GLButton    *addButton;
  GLButton    *cancelButton;
  GLLabel     *l1;
  GLLabel     *l2;
  GLLabel     *l3;
  GLTextField *x;
  GLTextField *y;
  GLTextField *z;

  int nbVertexS;


};

#endif /* _ADDVERTEXH_ */
