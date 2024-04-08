

/*
  File:        MirrorFacet.h
  Description: Mirror facet to plane dialog
*/
#ifndef _MIRRORFACETH_
#define _MIRRORFACETH_

#include "GLApp/GLWindow.h"
#include "Geometry_shared.h" //UndoPoint
#include <vector>

class InterfaceGeometry;
class Worker;
class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class MirrorFacet : public GLWindow {

public:
  // Construction
  MirrorFacet(InterfaceGeometry *interfGeom,Worker *work);
  void ClearUndoVertices();
  void ProcessMessage(GLComponent *src,int message) override;

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLButton     *mirrorButton,*mirrorCopyButton;
  GLButton    *projectButton,*projectCopyButton,*undoProjectButton;
  GLButton    *cancelButton;
  GLButton	  *getSelFacetButton;
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
  int    planeMode;
  std::vector<UndoPoint> undoPoints;

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

};

#endif /* _MirrorFacetH_ */
