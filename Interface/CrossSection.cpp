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

#include "Facet_shared.h"
#include "CrossSection.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry_shared.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#if defined(MOLFLOW)
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#endif

CrossSection::CrossSection(InterfaceGeometry* g, Worker* w, int viewerId_) :GLWindow() {

	int wD = 353;
	int hD = 260;
	planeDefPanel = new GLTitledPanel("Plane definition mode");
	planeDefPanel->SetBounds(12, 12, 326, 179);
	Add(planeDefPanel);
	label1 = new GLLabel("*X +");
	planeDefPanel->SetCompBounds(label1, 54, 45, 27, 13);
	planeDefPanel->Add(label1);

	eqmodeCheckbox = new GLToggle(0, "By equation");
	planeDefPanel->SetCompBounds(eqmodeCheckbox, 6, 19, 82, 17);
	planeDefPanel->Add(eqmodeCheckbox);

	XZplaneButton = new GLButton(0, "XZ plane");
	planeDefPanel->SetCompBounds(XZplaneButton, 182, 68, 82, 23);
	planeDefPanel->Add(XZplaneButton);

	YZplaneButton = new GLButton(0, "YZ plane");
	planeDefPanel->SetCompBounds(YZplaneButton, 94, 68, 82, 23);
	planeDefPanel->Add(YZplaneButton);

	XYplaneButton = new GLButton(0, "XY plane");
	planeDefPanel->SetCompBounds(XYplaneButton, 6, 68, 82, 23);
	planeDefPanel->Add(XYplaneButton);

	dTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(dTextbox, 249, 42, 42, 20);
	planeDefPanel->Add(dTextbox);

	label4 = new GLLabel("= 0");
	planeDefPanel->SetCompBounds(label4, 297, 45, 22, 13);
	planeDefPanel->Add(label4);

	cTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(cTextbox, 168, 42, 42, 20);
	planeDefPanel->Add(cTextbox);

	label3 = new GLLabel("*Z +");
	planeDefPanel->SetCompBounds(label3, 216, 45, 27, 13);
	planeDefPanel->Add(label3);

	bTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(bTextbox, 87, 42, 42, 20);
	planeDefPanel->Add(bTextbox);

	label2 = new GLLabel("*Y +");
	planeDefPanel->SetCompBounds(label2, 135, 45, 27, 13);
	planeDefPanel->Add(label2);

	aTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(aTextbox, 6, 42, 42, 20);
	planeDefPanel->Add(aTextbox);

	vertexModeCheckbox = new GLToggle(0, "By 3 selected vertex");
	planeDefPanel->SetCompBounds(vertexModeCheckbox, 6, 155, 122, 17);
	planeDefPanel->Add(vertexModeCheckbox);

	facetIdTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(facetIdTextbox, 107, 111, 61, 20);
	planeDefPanel->Add(facetIdTextbox);

	facetmodeCheckbox = new GLToggle(0, "Plane of facet:");
	planeDefPanel->SetCompBounds(facetmodeCheckbox, 6, 114, 95, 17);
	planeDefPanel->Add(facetmodeCheckbox);

	resultLabel = new GLLabel("resultLabel");
	resultLabel->SetBounds(15, 194, 58, 13);
	Add(resultLabel);

	sectionButton = new GLButton(0, "Section");
	sectionButton->SetBounds(12, 210, 129, 21);
	Add(sectionButton);

	invertButton = new GLButton(0, "Invert");
	invertButton->SetBounds(12, 210, 129, 21);
	Add(invertButton);

	cameraButton = new GLButton(0, "Camera");
	cameraButton->SetBounds(12, 210, 129, 21);
	Add(cameraButton);

	disableButton = new GLButton(0, "Disable");
	disableButton->SetBounds(202, 210, 129, 21);
	Add(disableButton);

	getSelectedFacetButton = new GLButton(0, "<- Get selected");
	planeDefPanel->SetCompBounds(getSelectedFacetButton, 174, 109, 90, 22);
	planeDefPanel->Add(getSelectedFacetButton);

	SetTitle("Cross section view");
	SetBounds(15, 30, wD, hD);
	RestoreDeviceObjects();

	resultLabel->SetText("");
	interfGeom = g;
	work = w;
	viewerId = viewerId_;
}

void CrossSection::ProcessMessage(GLComponent* src, int message) {
	double a, b, c, d;
	int facetNum;

	switch (message) {

	case MSG_TOGGLE:
		if (src == eqmodeCheckbox) planeMode = PlanemodeEquation;
		else if (src == facetmodeCheckbox) planeMode = PlanemodeFacet;
		else if (src == vertexModeCheckbox) planeMode = Planemode3Vertex;
		EnableDisableControls(planeMode);
		break;

	case MSG_BUTTON:

		if (src == XYplaneButton) {
			aTextbox->SetText(0);
			bTextbox->SetText(0);
			cTextbox->SetText(1);
			dTextbox->SetText(0);
			planeMode = PlanemodeEquation;
			EnableDisableControls(planeMode);
		}
		else if (src == YZplaneButton) {
			aTextbox->SetText(1);
			bTextbox->SetText(0);
			cTextbox->SetText(0);
			dTextbox->SetText(0);
			planeMode = PlanemodeEquation;
			EnableDisableControls(planeMode);
		}
		else if (src == XZplaneButton) {
			aTextbox->SetText(0);
			bTextbox->SetText(1);
			cTextbox->SetText(0);
			dTextbox->SetText(0);
			planeMode = PlanemodeEquation;
			EnableDisableControls(planeMode);
		}
		else if (src == getSelectedFacetButton) {
			if (interfGeom->GetNbSelectedFacets() != 1) {
				GLMessageBox::Display("Select exactly one facet.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			int selFacetId = -1;
			for (int i = 0; selFacetId == -1 && i < interfGeom->GetNbFacet(); i++) {
				if (interfGeom->GetFacet(i)->selected) {
					selFacetId = i;
				}
			}
			facetIdTextbox->SetText(selFacetId + 1);
			EnableDisableControls(planeMode = PlanemodeFacet);
		}
		else if (src == disableButton) {
			auto myView = mApp->viewer[viewerId]->GetCurrentView();
			myView.enableClipping = false;
			mApp->viewer[viewerId]->SetCurrentView(myView);
		}
		else if (src == sectionButton) {
			//Calculate the plane
			Vector3d P0, N;
			double nN2;

			switch (planeMode) {
			case PlanemodeFacet:
				if (!(facetIdTextbox->GetNumberInt(&facetNum)) || facetNum<1 || facetNum>interfGeom->GetNbFacet()) {
					GLMessageBox::Display("Invalid facet number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				P0 = *interfGeom->GetVertex(interfGeom->GetFacet(facetNum - 1)->indices[0]);
				N = interfGeom->GetFacet(facetNum - 1)->sh.N;
				break;
			case Planemode3Vertex:
			{
				auto selectedVertexIds = interfGeom->GetSelectedVertices();
				if (selectedVertexIds.size() != 3) {
					GLMessageBox::Display("Select exactly 3 vertices", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}

				Vector3d U2 = (*interfGeom->GetVertex(selectedVertexIds[0]) - *interfGeom->GetVertex(selectedVertexIds[1])).Normalized();
				Vector3d V2 = (*interfGeom->GetVertex(selectedVertexIds[0]) - *interfGeom->GetVertex(selectedVertexIds[2])).Normalized();
				Vector3d N2 = CrossProduct(V2, U2); //We have a normal vector
				nN2 = N2.Norme();
				if (nN2 < 1e-8) {
					GLMessageBox::Display("The 3 selected vertices are on a line.", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				; // Normalize N2
				N = N2 * (1.0 / nN2);
				P0 = *(interfGeom->GetVertex(selectedVertexIds[0]));
				break;
			}
			case PlanemodeEquation:
				if (!(aTextbox->GetNumber(&a))) {
					GLMessageBox::Display("Invalid A coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(bTextbox->GetNumber(&b))) {
					GLMessageBox::Display("Invalid B coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(cTextbox->GetNumber(&c))) {
					GLMessageBox::Display("Invalid C coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(dTextbox->GetNumber(&d))) {
					GLMessageBox::Display("Invalid D coefficient", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if ((a == 0.0) && (b == 0.0) && (c == 0.0)) {
					GLMessageBox::Display("A, B, C are all zero. That's not a plane.", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				N.x = a; N.y = b; N.z = c;
				P0.x = 0.0; P0.y = 0; P0.z = 0;
				if (a != 0) P0.x = -d / a;
				else if (b != 0) P0.y = -d / b;
				else if (c != 0) P0.z = -d / c;
				break;
			default:
				GLMessageBox::Display("Select a plane definition mode.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			if (mApp->AskToReset()) {
				//Set plane
				lastPlane.d = -Dot(N, P0);
				Vector3d N_normalized = N.Normalized();
				lastPlane.a = N_normalized.x;
				lastPlane.b = N_normalized.y;
				lastPlane.c = N_normalized.z;
				auto myView = mApp->viewer[viewerId]->GetCurrentView();
				myView.enableClipping = true;
				myView.clipPlane = lastPlane;
				mApp->viewer[viewerId]->SetCurrentView(myView);
			}
		}
		else if (src == invertButton) {
			lastPlane.d *= -1.0;
			auto myView = mApp->viewer[viewerId]->GetCurrentView();
			myView.clipPlane = lastPlane;
			mApp->viewer[viewerId]->SetCurrentView(myView);
		}
		else if (src == cameraButton) {

		}
		break;
	case MSG_TEXT_UPD:
		if (src == aTextbox || src == bTextbox || src == cTextbox || src == dTextbox) {
			planeMode = PlanemodeEquation;
			EnableDisableControls(planeMode);
		}
		else if (src == facetIdTextbox) {
			planeMode = PlanemodeFacet;
			EnableDisableControls(planeMode);
		}
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

void CrossSection::EnableDisableControls(int mode) {
	eqmodeCheckbox->SetState(mode == PlanemodeEquation);
	facetmodeCheckbox->SetState(mode == PlanemodeFacet);
	vertexModeCheckbox->SetState(mode == Planemode3Vertex);
}