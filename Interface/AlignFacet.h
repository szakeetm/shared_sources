
#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;
class Vector3d;

#include <vector> //Std vectors

class InterfaceGeometry;
class Worker;

class AlignFacet : public GLWindow {

public:

  // Construction
  AlignFacet(InterfaceGeometry *interfGeom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;
  void MemorizeSelection();

private:

  InterfaceGeometry     *interfGeom;
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
