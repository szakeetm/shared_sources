/*
  File:        AlignFacet.h
  Description: Move facet by offset dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _AlignFacetH_
#define _AlignFacetH_

class AlignFacet : public GLWindow {

public:

  // Construction
  AlignFacet(Geometry *geom,Worker *work);
  ~AlignFacet();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void MemorizeSelection();

private:

  Geometry     *geom;
  Worker	   *work;

  int*        selection;
  int         nbMemo;
  int         nbSelected;

  VERTEX3D    **oriPos;

  GLButton    *memoSel;
  GLLabel     *numFacetSel;
  GLButton    *alignButton;
  GLButton    *copyButton;
  GLButton    *undoButton;
  
  GLButton    *cancelButton;

  GLLabel     *l1;
  GLToggle    *invertNormal;
  GLToggle    *invertDir1;
  GLToggle    *invertDir2;

  GLTitledPanel *step1;
  GLTitledPanel *step2;
  GLTitledPanel *step3;

  int nbFacetS;

};

#endif /* _AlignFacetH_ */
