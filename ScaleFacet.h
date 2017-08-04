/*
  File:        ScaleFacet.h
  Description: Mirror facet to plane dialog
*/
#ifndef _ScaleFacetH_
#define _ScaleFacetH_

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class Geometry;
class Worker;

class ScaleFacet : public GLWindow {

public:
  // Construction
  ScaleFacet(Geometry *geom,Worker *work);
  void ProcessMessage(GLComponent *src,int message);

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLTitledPanel *sPanel;
  GLButton     *scaleButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLButton	  *getSelFacetButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLTextField *xText;
  GLTextField *yText;
  GLTextField *zText;
  GLTextField *facetNumber;
  GLTextField *factorNumber;
  GLTextField *OnePerFactor;
  GLTextField *factorNumberX;
  GLTextField *factorNumberY;
  GLTextField *factorNumberZ;
  GLToggle    *uniform;
  GLToggle    *distort;

  int nbFacetS,invariantMode,scaleMode;

  Geometry     *geom;
  Worker	   *work;

};

#endif /* _ScaleFacetH_ */
