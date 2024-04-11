/*
  File:        SplitFacet.h
  Description: Split facet by plane dialog
*/

#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class InterfaceGeometry;
class Worker;

class SplitFacet : public GLWindow {

public:
  // Construction
  SplitFacet(InterfaceGeometry *interfGeom,Worker *work);
  ~SplitFacet();
  void ProcessMessage(GLComponent *src,int message) override;
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
  GLButton	*getSelectedFacetButton;

  enum PlaneMode : int {
	  PlanemodeEquation,
	  PlanemodeFacet,
	  Planemode3Vertex
  };
  PlaneMode planeMode=PlanemodeEquation;

  std::vector<DeletedFacet> deletedFacetList;
  size_t nbFacet{ 0 }, nbCreated{ 0 };

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

};
