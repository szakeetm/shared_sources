/*
  File:        MirrorVertex.h
  Description: Mirror vertex to plane dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _MirrorVertexH_
#define _MirrorVertexH_

class MirrorVertex : public GLWindow {

public:
  // Construction
  MirrorVertex(Geometry *geom,Worker *work);
  void ClearUndoVertices();
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLButton     *mirrorButton, *mirrorCopyButton;
  GLButton    *projectButton, *projectCopyButton, *undoProjectButton;
  GLButton    *cancelButton;
  GLButton	  *getPlaneButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLToggle     *l4;
  GLToggle     *l6;
  GLTextField *aText;
  GLTextField *bText;
  GLTextField *cText;
  GLTextField *dText;
  GLLabel		*aLabel;
  GLLabel		*bLabel;
  GLLabel		*cLabel;
  GLLabel		*dLabel;

  int nbFacetS;
  int    planeMode;
  std::vector<UndoPoint> undoPoints;

  Geometry     *geom;
  Worker	   *work;

};

#endif /* _MirrorVertexH_ */
