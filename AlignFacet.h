#pragma once
/*
  File:        AlignFacet.h
  Description: Move facet by offset dialog
*/

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;
class Vector3d;

#include <vector> //Std vectors

class Geometry;
class Worker;

class AlignFacet : public GLWindow {

public:

  // Construction
  AlignFacet(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  void MemorizeSelection();

private:

  Geometry     *geom;
  Worker	   *work;

  std::vector<size_t> memorizedSelection;
  std::vector<std::vector<Vector3d>> oriPositions;

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

};
