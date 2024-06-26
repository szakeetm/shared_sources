
#pragma once
/*
  File:        ExtrudeFacet.h
  Description: Extrude facet by offset dialog
*/

#include "GLApp/GLWindow.h"
#include <optional>
class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

//#include "Geometry_shared.h"
//#include "Worker.h"
class InterfaceGeometry;
class Worker;

class ExtrudeFacet : public GLWindow {

public:

  // Construction
  ExtrudeFacet(InterfaceGeometry *interfGeom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

  GLToggle	*offsetCheckbox;
  GLLabel	*label3;
  GLTextField	*dxText;
  GLLabel	*label4;
  GLLabel	*label5;
  GLLabel	*label6;
  GLTextField	*dyText;
  GLLabel	*label7;
  GLLabel	*label8;
  GLTextField	*dzText;
  GLButton	*extrudeButton;
  GLButton	*getBaseButton;
  GLButton	*getDirButton;
  GLToggle	*towardsNormalCheckbox;
  GLToggle	*againstNormalCheckbox;
  GLLabel	*label1;
  GLTextField	*distanceTextbox;
  GLLabel	*label2;
  GLTitledPanel	*groupBox1;
  GLTitledPanel	*groupBox2;
  GLLabel	*dirLabel;
  GLLabel	*baseLabel;
  GLTitledPanel	*groupBox3;
  GLButton	*curveGetDirButton;
  GLButton	*curveGetBaseButton;
  GLLabel	*label11;
  GLLabel	*label12;
  GLTextField	*curvedZText;
  GLLabel	*label13;
  GLLabel	*label14;
  GLTextField	*curvedYText;
  GLLabel	*label15;
  GLLabel	*label16;
  GLTextField	*curvedXText;
  GLLabel	*label17;
  GLLabel	*label23;
  GLTextField	*curveRadiusLengthText;
  GLToggle	*curveAgainstNormalCheckbox;
  GLToggle	*curveTowardsNormalCheckbox;
  GLLabel	*label18;
  GLTextField	*curveZ0Text;
  GLLabel	*label19;
  GLLabel	*label20;
  GLTextField	*curveY0Text;
  GLLabel	*label21;
  GLLabel	*label22;
  GLTextField	*curveX0Text;
  GLLabel	*label24;
  GLTextField	*curveTotalAngleDegText;
  GLLabel	*label25;
  GLTextField	*curveStepsText;
  GLLabel	*label26;
  GLLabel	*label27;
  GLTextField	*curveTotalAngleRadText;
  GLLabel	*label10;
  GLLabel	*label9;
  GLButton	*curveFacetVButton;
  GLButton	*curveFacetUButton;
  GLButton	*curveFacetIndex1Button;
  GLButton	*curveFacetCenterButton;
  GLLabel	*label28;
  GLLabel	*curveDirLabel;
  GLLabel	*curveBaseLabel;
  GLButton	*facetNYbutton;
  GLButton	*facetNXbutton;
  GLButton	*facetNZbutton;
  GLLabel	*label30;
  GLLabel	*label29;
  GLTextField	*curveTotalLengthText;
	
  size_t baseId, dirId;
  void EnableDisableControls();
  void ClearToggles(GLToggle* leaveChecked=NULL);
  std::optional<size_t> AssertOneVertexSelected();
  std::optional<size_t> AssertOneFacetSelected();
};
