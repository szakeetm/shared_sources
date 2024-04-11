
#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;
class GLScrollBar;
class GLToggle;

class InterfaceGeometry;
class Worker;

class CrossSection : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
	CrossSection(InterfaceGeometry* g, Worker* w, int viewerId_);
	void SetViewer(int viewerId_);
  void ProcessMessage(GLComponent *src,int message) override;
private:

	GLTitledPanel* planeDefPanel;

	GLLabel* label1;
	GLLabel* label2;
	GLLabel* label3;
	GLLabel* label4;

	GLLabel* DminLabel;
	GLLabel* DmaxLabel;

	GLLabel* hintLabel;

	GLTextField* aTextbox;
	GLTextField* bTextbox;
	GLTextField* cTextbox;
	GLTextField* dTextbox;

	GLButton* XZplaneButton;
	GLButton* YZplaneButton;
	GLButton* XYplaneButton;
	
	GLButton* selectedFacetButton;
	GLButton* selectedVertexButton;
	GLButton* cameraButton;

	GLScrollBar* dScrollBar;
	
	GLToggle* enableToggle;
	GLButton* invertButton;

	InterfaceGeometry* interfGeom;
	Worker* work;
	int viewerId;

	Plane ReadTextboxValues(); //throws error
	void FillTextboxValues(const Plane& p);
	void AdjustScrollbar(const Plane& p);
	void Refresh();

	double Dmin, Dmax;
};
