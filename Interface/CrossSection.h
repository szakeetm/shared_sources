/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;
class GLScrollBar;

class InterfaceGeometry;
class Worker;

class CrossSection : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
	CrossSection(InterfaceGeometry* g, Worker* w, int viewerId_);
  void ProcessMessage(GLComponent *src,int message) override;
private:

	GLTitledPanel* planeDefPanel;

	GLLabel* label1;
	GLLabel* label2;
	GLLabel* label3;
	GLLabel* label4;

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
	
	GLButton* sectionButton;
	GLButton* invertButton;
	GLButton* disableButton;

	InterfaceGeometry* interfGeom;
	Worker* work;
	int viewerId;

	Plane GetPlane(); //throws error
	void SetPlane(const Plane& p);
};
