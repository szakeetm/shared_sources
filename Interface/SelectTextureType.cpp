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
#include "SelectTextureType.h"
#include "GLApp/GLMessageBox.h"
#include "Facet_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTextField.h"
#include "Helper/MathTools.h" //IsEqual
#include "GLApp/GLButton.h"

#include "Geometry_shared.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

// Construct a message dialog box
SelectTextureType::SelectTextureType(Worker *w) :GLWindow() {

	int wD, hD;
	wD = 280; hD = 260;
	SetTitle("Select facets by texture properties");

	resolutionpanel = new GLTitledPanel("Texture resolution");
	resolutionpanel->SetBounds(5, 5, wD - 10, 120);
	resolutionpanel->SetClosable(false);
	Add(resolutionpanel);

	squareToggle = new GLToggle(0, "Square texture");
	squareToggle->AllowMixedState(true);
	squareToggle->SetState(2);
	resolutionpanel->Add(squareToggle);
	resolutionpanel->SetCompBounds(squareToggle, 5, 16, 260, 20);

	GLLabel* ratioInfo = new GLLabel("For non-square textures, condition applies to either\nof the two dimensions:");
	resolutionpanel->Add(ratioInfo);
	resolutionpanel->SetCompBounds(ratioInfo, 7, 32, 260, 20);


	ratioToggle = new GLToggle(0, "Exactly                   /cm");
	resolutionpanel->Add(ratioToggle);
	resolutionpanel->SetCompBounds(ratioToggle, 5, 66, 150, 20);

	ratioText = new GLTextField(0, "");
	resolutionpanel->Add(ratioText);
	resolutionpanel->SetCompBounds(ratioText, 70, 65, 60, 19);

	ratioMinMaxToggle = new GLToggle(1, "Between                 /cm and                  /cm");
	resolutionpanel->Add(ratioMinMaxToggle);
	resolutionpanel->SetCompBounds(ratioMinMaxToggle, 5, 92, 250, 20);

	ratioMinText = new GLTextField(0, "");
	resolutionpanel->Add(ratioMinText);
	resolutionpanel->SetCompBounds(ratioMinText, 70, 92, 60, 19);

	ratioMaxText = new GLTextField(0, "");
	resolutionpanel->Add(ratioMaxText);
	resolutionpanel->SetCompBounds(ratioMaxText, 180, 92, 60, 19);

	textureTypePanel = new GLTitledPanel("Texture type");
	textureTypePanel->SetBounds(5, 130, wD - 10, 80);
	textureTypePanel->SetClosable(false);
	Add(textureTypePanel);

	desorbToggle = new GLToggle(0, "Count desorbtion");
	desorbToggle->AllowMixedState(true);
	desorbToggle->SetState(2);
	textureTypePanel->Add(desorbToggle);
	textureTypePanel->SetCompBounds(desorbToggle, 5, 15, 90, 18);

	absorbToggle = new GLToggle(0, "Count absorbtion");
	absorbToggle->AllowMixedState(true);
	absorbToggle->SetState(2);
	textureTypePanel->Add(absorbToggle);
	textureTypePanel->SetCompBounds(absorbToggle, 5, 35, 90, 18);

	reflectToggle = new GLToggle(0, "Count reflection");
	reflectToggle->AllowMixedState(true);
	reflectToggle->SetState(2);
	textureTypePanel->Add(reflectToggle);
	textureTypePanel->SetCompBounds(reflectToggle, 130, 15, 90, 18);

	transparentToggle = new GLToggle(0, "Count transp. pass");
	transparentToggle->AllowMixedState(true);
	transparentToggle->SetState(2);
	textureTypePanel->Add(transparentToggle);
	textureTypePanel->SetCompBounds(transparentToggle, 130, 35, 80, 18);

	directionToggle = new GLToggle(0, "Count direction");
	directionToggle->AllowMixedState(true);
	directionToggle->SetState(2);
	textureTypePanel->Add(directionToggle);
	textureTypePanel->SetCompBounds(directionToggle, 130, 55, 80, 18);

	// Buttons
	int startX = 5;
	int startY = 215;

	selectButton = new GLButton(0, "Select");
	selectButton->SetBounds(startX, startY, 75, 20);
	Add(selectButton);
	startX += 80;

	addSelectButton = new GLButton(0, "Add to sel.");
	addSelectButton->SetBounds(startX, startY, 95, 20);
	Add(addSelectButton);
	startX += 100;

	remSelectButton = new GLButton(GLDLG_SELECT_REM, "Remove from sel.");
	remSelectButton->SetBounds(startX, startY, 90, 20);
	Add(remSelectButton);

	// Top left
	SetBounds(10, 30, wD, hD);

	// Create objects
	RestoreDeviceObjects();

	work = w;
	interfGeom = w->GetGeometry();
}

void SelectTextureType::ProcessMessage(GLComponent *src, int message) {
	if (message == MSG_BUTTON) {
		double ratio, minRatio, maxRatio;
		bool exactRatio = false;
		bool minmaxRatio = false;
		if (ratioToggle->GetState()) {
			if (!ratioText->GetNumber(&ratio) || ratio <= 0.0) {
				GLMessageBox::Display("Invalid ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			exactRatio = true;
		}
		else if (ratioMinMaxToggle->GetState()) {
			if (!ratioMinText->GetNumber(&minRatio) || minRatio < 0.0) {
				GLMessageBox::Display("Invalid min. ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			if (!ratioMaxText->GetNumber(&maxRatio) || maxRatio <= 0.0) {
				GLMessageBox::Display("Invalid max. ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			if (maxRatio <= minRatio) {
				GLMessageBox::Display("Max. ratio must be larger than min. ratio", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			minmaxRatio = true;
		}
		if (src == selectButton) interfGeom->UnselectAll();
		for (size_t i = 0; i < interfGeom->GetNbFacet(); i++) {
			InterfaceFacet* f = interfGeom->GetFacet(i);
			bool match = f->sh.isTextured;
			if (squareToggle->GetState() != 2) match = match && ((squareToggle->GetState()==1) == IsEqual(f->tRatioU,f->tRatioV));
			if (exactRatio) match = match && IsEqual(ratio, f->tRatioU) || IsEqual(ratio, f->tRatioV);
			if (minmaxRatio) match = match && ((minRatio <= f->tRatioU) && (f->tRatioU <= maxRatio)) || ((minRatio <= f->tRatioV) && (f->tRatioV <= maxRatio));
#if defined(MOLFLOW)
			if (desorbToggle->GetState() != 2) match = match && f->sh.countDes;
#endif
			if (absorbToggle->GetState() != 2) match = match && (absorbToggle->GetState()==1) == f->sh.countAbs;
			if (reflectToggle->GetState() != 2) match = match && (reflectToggle->GetState()==1) == f->sh.countRefl;
			if (transparentToggle->GetState() != 2) match = match && (transparentToggle->GetState()==1) == f->sh.countTrans;
			if (directionToggle->GetState() != 2) match = match && (directionToggle->GetState()==1) && f->sh.countDirection;

			if (match) f->selected = (src != remSelectButton);
		}
		interfGeom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected();
	}
	else if (message == MSG_TOGGLE) {
		if (src == ratioToggle) ratioMinMaxToggle->SetState(false);
		if (src == ratioMinMaxToggle) ratioToggle->SetState(false);
	}
	else if (message == MSG_TEXT_UPD) {
		if (src == ratioText) {
			ratioToggle->SetState(1);
			ratioMinMaxToggle->SetState(0);
		}
		else {
			ratioToggle->SetState(0);
			ratioMinMaxToggle->SetState(1);
		}
	}
	GLWindow::ProcessMessage(src, message);
}