/*
  File:        SplitFacet.h
  Description: Split facet by plane dialog
*/

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"

#ifndef _SPLITFACETH_
#define _SPLITFACETH_

class SplitFacet : public GLWindow {

public:
  // Construction
  SplitFacet(Geometry *geom,Worker *work);
  ~SplitFacet();
  void ProcessMessage(GLComponent *src,int message);
  void ClearUndoFacets();

  // Implementation
private:

  void EnableDisableControls(int mode);
  
  GLTitledPanel	*planeDefPanel;
  GLLabel	*label1;
  GLToggle	*eqmodeCheckbox;
  GLButton	*XZplaneButton;
  GLButton	*YZplaneButton;
  GLButton	*XYplaneButton;
  GLTextField	*dTextbox;
  GLLabel	*label4;
  GLTextField	*cTextbox;
  GLLabel	*label3;
  GLTextField	*bTextbox;
  GLLabel	*label2;
  GLTextField	*aTextbox;
  GLToggle	*vertexModeCheckbox;
  GLTextField	*facetIdTextbox;
  GLToggle	*facetmodeCheckbox;
  GLLabel	*resultLabel;
  GLButton	*splitButton;
  GLButton	*undoButton;

  int planeMode;
  std::vector<DeletedFacet> deletedFacetList;
  size_t nbFacet, nbCreated;

  Geometry     *geom;
  Worker	   *work;


};

#endif /* _SplitFacetH_ */
