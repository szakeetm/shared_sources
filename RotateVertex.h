/*
File:        RotateVertex.h
Description: Rotate vertex around axis dialog
*/
#pragma once

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry.h"
#include "Worker.h"



class RotateVertex : public GLWindow {

public:
	// Construction
	RotateVertex(Geometry *geom, Worker *work);
	void ProcessMessage(GLComponent *src, int message);

	// Implementation
private:

	void UpdateToggle(GLComponent *src);

	GLTitledPanel *iPanel;
	GLButton     *moveButton, *copyButton, *cancelButton, *getBaseVertexButton, *getDirVertexButton;
	GLToggle     *l1;
	GLToggle     *l2;
	GLToggle     *l3;
	GLToggle     *l4;
	GLToggle     *l5;
	GLToggle     *l6;
	GLLabel     *lNum;
	
	GLToggle     *l8;
	GLTextField *aText;
	GLTextField *bText;
	GLTextField *cText;
	GLTextField *uText;
	GLTextField *vText;
	GLTextField *wText;
	
	GLTextField *degText, *radText;
	GLLabel		*aLabel;
	GLLabel		*bLabel;
	GLLabel		*cLabel;
	GLLabel		*uLabel;
	GLLabel		*vLabel;
	GLLabel		*wLabel;
	GLLabel		*degLabel, *radLabel;

	int nbFacetS;
	int    axisMode;

	Geometry   *geom;
	Worker	   *work;


};

