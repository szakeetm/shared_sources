#pragma once

#include "GLApp/GLWindow.h"
#include "Vector.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class Geometry;
class Worker;
class GLTitledPanel;

class MoveVertex : public GLWindow {

public:

	// Construction
	MoveVertex(Geometry *geom, Worker *work);

	// Implementation
	void ProcessMessage(GLComponent *src, int message);

private:

	Geometry     *geom;
	Worker	   *work;

	GLToggle	*offsetCheckbox;
	GLToggle	*directionCheckBox;
	GLTextField	*distanceText;
	GLLabel	*dxLabel;
	GLTextField	*xText;
	GLLabel	*cmLabelX;
	GLLabel	*cmLabelY;
	GLTextField	*yText;
	GLLabel	*dyLabel;
	GLLabel	*cmLabelZ;
	GLTextField	*zText;
	GLLabel	*dzLabel;
	GLTitledPanel	*directionPanel;
	GLButton	*dirFacetCenterButton;
	GLButton	*dirVertexButton;
	GLButton	*baseFacetCenterButton;
	GLButton	*baseVertexButton;
	GLButton	*facetNormalButton;
	GLLabel	*label4;
	GLLabel	*label1;
	GLLabel	*directionStatusLabel;
	GLLabel	*baseStatusLabel;
	GLButton	*copyButton;
	GLButton	*moveButton;
	GLTitledPanel	*dirPanel;
	GLTitledPanel	*basePanel;

	Vector3d baseLocation;

};