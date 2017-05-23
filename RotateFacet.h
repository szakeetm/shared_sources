/*
  File:        RotateFacet.h
  Description: Rotate facet around axis dialog
*/
#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class Geometry;
class Worker;

class RotateFacet : public GLWindow {

public:
  // Construction
  RotateFacet(Geometry *geom,Worker *work);
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLButton     *moveButton,*copyButton,*cancelButton,*getSelFacetButton,*getBaseVertexButton,*getDirVertexButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLToggle     *l4;
  GLToggle     *l5;
  GLToggle     *l6;
  GLLabel     *lNum;
  GLToggle     *l7;
  GLToggle     *l8;
  GLTextField *aText;
  GLTextField *bText;
  GLTextField *cText;
  GLTextField *uText;
  GLTextField *vText;
  GLTextField *wText;
  GLTextField *facetNumber;
  GLTextField *degText,*radText;
  GLLabel		*aLabel;
  GLLabel		*bLabel;
  GLLabel		*cLabel;
  GLLabel		*uLabel;
  GLLabel		*vLabel;
  GLLabel		*wLabel;
  GLLabel		*degLabel,*radLabel;

  int nbFacetS;
  int    axisMode;

  Geometry     *geom;
  Worker	   *work;


};

