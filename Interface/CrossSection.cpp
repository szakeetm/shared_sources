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
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLScrollBar.h"

#include "Helper/MathTools.h" //Saturate
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

	int wD = 420;
	int hD = 200;

	interfGeom = g;
	work = w;
	viewerId = viewerId_;
	
	SetTitle(fmt::format("Cross section view (Viewer #{})", viewerId + 1));
	SetBounds(15, 30, wD, hD);

	planeDefPanel = new GLTitledPanel("Cut plane");
	planeDefPanel->SetBounds(10, 7, wD - 17, 135);
	Add(planeDefPanel);

	int panelPosX = 8;
	int panelPosY = 15;

	int elementWidth = 60;
	int elementSpacing = 5;
	int elementHeight = 20;
	aTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(aTextbox, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(aTextbox);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 22;
	label1 = new GLLabel("*X +");
	planeDefPanel->SetCompBounds(label1, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(label1);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 65;
	bTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(bTextbox, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(bTextbox);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 22;
	label2 = new GLLabel("*Y +");
	planeDefPanel->SetCompBounds(label2, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(label2);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 65;
	cTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(cTextbox, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(cTextbox);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 22;
	label3 = new GLLabel("*Z +");
	planeDefPanel->SetCompBounds(label3, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(label3);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 65;
	dTextbox = new GLTextField(0, "");
	planeDefPanel->SetCompBounds(dTextbox, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(dTextbox);
	panelPosX += elementWidth + elementSpacing;

	elementWidth = 22;
	label4 = new GLLabel("= 0");
	planeDefPanel->SetCompBounds(label4, panelPosX, panelPosY, 22, elementHeight);
	planeDefPanel->Add(label4);

	 //newline
	panelPosY += 25;
	panelPosX = 8;

	elementWidth = 120;
	elementSpacing = 15;
	
	XYplaneButton = new GLButton(0, "XY plane");
	planeDefPanel->SetCompBounds(XYplaneButton, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(XYplaneButton);
	panelPosX += elementWidth + elementSpacing;

	XZplaneButton = new GLButton(0, "XZ plane");
	planeDefPanel->SetCompBounds(XZplaneButton, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(XZplaneButton);
	panelPosX += elementWidth + elementSpacing;

	YZplaneButton = new GLButton(0, "YZ plane");
	planeDefPanel->SetCompBounds(YZplaneButton, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(YZplaneButton);
	panelPosX += elementWidth + elementSpacing;

	//newline
	panelPosY += 25;
	panelPosX = 8;

	selectedFacetButton = new GLButton(0, "Selected facet");
	planeDefPanel->SetCompBounds(selectedFacetButton, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(selectedFacetButton);
	panelPosX += elementWidth + elementSpacing;

	selectedVertexButton = new GLButton(0, "Selected 3 vertices");
	planeDefPanel->SetCompBounds(selectedVertexButton, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(selectedVertexButton);
	panelPosX += elementWidth + elementSpacing;

	cameraButton = new GLButton(0, "Camera midplane");
	planeDefPanel->SetCompBounds(cameraButton, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(cameraButton);
	panelPosX += elementWidth + elementSpacing;

	//newline
	panelPosY += 25;
	panelPosX = 8;

	elementWidth = wD - 155;
	elementHeight = 15;
	dScrollBar = new GLScrollBar(0);
	dScrollBar->SetOrientation(SB_HORIZONTAL);
	dScrollBar->SetEnabled(false); //Enabled by AdjustScrollbar()
	planeDefPanel->SetCompBounds(dScrollBar, panelPosX, panelPosY, wD-35, elementHeight);
	dScrollBar->SetRange(1100, 100, 1);
	planeDefPanel->Add(dScrollBar);
	panelPosX += elementWidth + elementSpacing;
	
	//newline
	panelPosY += 20;
	panelPosX = 8;

	elementWidth = 100;
	elementHeight = 20;
	DminLabel = new GLLabel("D_min");
	planeDefPanel->SetCompBounds(DminLabel, panelPosX, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(DminLabel);

	DmaxLabel = new GLLabel("D_max");
	planeDefPanel->SetCompBounds(DmaxLabel, wD-elementWidth-20, panelPosY, elementWidth, elementHeight);
	planeDefPanel->Add(DmaxLabel);

	//newline
	panelPosY += 40;
	panelPosX = 12;

	elementWidth = 120;
	elementHeight = 20;
	sectionButton = new GLButton(0, "Section");
	sectionButton->SetBounds(panelPosX, panelPosY, elementWidth, elementHeight);
	Add(sectionButton);
	panelPosX += elementWidth + elementSpacing;

	invertButton = new GLButton(0, "Invert");
	invertButton->SetBounds(panelPosX, panelPosY, elementWidth, elementHeight);
	Add(invertButton);
	panelPosX += elementWidth + elementSpacing;

	disableButton = new GLButton(0, "Disable");
	disableButton->SetBounds(panelPosX, panelPosY, elementWidth, elementHeight);
	Add(disableButton);

	RestoreDeviceObjects();
}

void CrossSection::ProcessMessage(GLComponent* src, int message) {

	switch (message) {

	case MSG_BUTTON:

		if (src == XYplaneButton) {
			Plane clipPlane = Plane(0.0, 0.0, 1.0, 0.0);
			mApp->viewer[viewerId]->ApplyClippingPlane(clipPlane, true);
			FillTextboxValues(clipPlane);
			AdjustScrollbar(clipPlane);
		}
		else if (src == YZplaneButton) {
			Plane clipPlane = Plane(1.0, 0.0, 0.0, 0.0);
			mApp->viewer[viewerId]->ApplyClippingPlane(clipPlane, true);
			FillTextboxValues(clipPlane);
			AdjustScrollbar(clipPlane);
		}
		else if (src == XZplaneButton) {
			Plane clipPlane = Plane(0.0, 1.0, 0.0, 0.0);
			mApp->viewer[viewerId]->ApplyClippingPlane(clipPlane, true);
			FillTextboxValues(clipPlane);
			AdjustScrollbar(clipPlane);
		}
		else if (src == selectedFacetButton) {
			auto selFacets = interfGeom->GetSelectedFacets();
			if (selFacets.size() != 1) {
				GLMessageBox::Display("Select exactly one facet.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			const Vector3d& P0 = *interfGeom->GetVertex(interfGeom->GetFacet(selFacets[0])->indices[0]);
			const Vector3d& N = interfGeom->GetFacet(selFacets[0])->sh.N;
			Plane facetPlane = Plane(N.x, N.y, N.z, -Dot(N, P0));
			mApp->viewer[viewerId]->ApplyClippingPlane(facetPlane, true);
			FillTextboxValues(facetPlane);
			AdjustScrollbar(facetPlane);
			break;
		}
		else if (src == selectedVertexButton) {
			auto selVertices = interfGeom->GetSelectedVertices();
			if (selVertices.size() != 3) {
				GLMessageBox::Display("Select exactly three vertices.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}

			auto v0 = *interfGeom->GetVertex(selVertices[0]);
			auto v1 = *interfGeom->GetVertex(selVertices[1]);
			auto v2 = *interfGeom->GetVertex(selVertices[2]);
			Vector3d U2 = (v0 - v1).Normalized();
			Vector3d V2 = (v0 - v2).Normalized();
			Vector3d N2 = CrossProduct(V2, U2); //We have a normal vector
			double nN2 = N2.Norme();
			if (nN2 < 1e-8) {
				GLMessageBox::Display("The 3 selected vertices are on a line.", "Can't define plane", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			// Normalize N2
			Vector3d N = N2.Normalized();
			Plane vertexPlane = Plane(N.x, N.y, N.z, -Dot(N, v0));
			mApp->viewer[viewerId]->ApplyClippingPlane(vertexPlane, true);
			FillTextboxValues(vertexPlane);
			AdjustScrollbar(vertexPlane);
			break;
		}
		else if (src == disableButton) {
			mApp->viewer[viewerId]->ApplyClippingPlane(std::nullopt, false);
		}
		else if (src == sectionButton) {
			Plane equationPlane;
			try {
				equationPlane = ReadTextboxValues();
			}
			catch (Error& err) {
				GLMessageBox::Display(err.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			mApp->viewer[viewerId]->ApplyClippingPlane(equationPlane, true);
			AdjustScrollbar(equationPlane);
		}
		else if (src == invertButton) {
			Plane cutPlane;
			try {
				cutPlane = ReadTextboxValues();
			}
			catch (Error& err) {
				GLMessageBox::Display(err.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}
			Plane invertedPlane = Plane(-cutPlane.a, -cutPlane.b, -cutPlane.c, -cutPlane.d);
			mApp->viewer[viewerId]->ApplyClippingPlane(invertedPlane, std::nullopt);
			FillTextboxValues(invertedPlane);
			AdjustScrollbar(invertedPlane);
		}
		else if (src == cameraButton) {
			Plane cameraPlane = mApp->viewer[viewerId]->GetCameraPlane();
			mApp->viewer[viewerId]->ApplyClippingPlane(cameraPlane, true);
			FillTextboxValues(cameraPlane);
			AdjustScrollbar(cameraPlane);
		}
		break;
	case MSG_SCROLL:
	{
		Plane cutPlane;
		try {
			cutPlane = ReadTextboxValues();
		}
		catch (Error& err) {
			GLMessageBox::Display(err.what(), "Error", GLDLG_OK, GLDLG_ICONERROR);
			return;
		}
		int pos = dScrollBar->GetPosition();
		double D_range = Dmax - Dmin;
		double d = Dmin + static_cast<double>(pos) / 1000.0 * D_range;
		cutPlane.d = d;
		mApp->viewer[viewerId]->ApplyClippingPlane(cutPlane, true);
		FillTextboxValues(cutPlane);
		break;
	}
	}
	GLWindow::ProcessMessage(src, message);
}

void CrossSection::AdjustScrollbar(const Plane& p)
{
	// Get min and max D values where plane still intersects bounding box
	AxisAlignedBoundingBox BB = interfGeom->GetBB();
	std::vector<Vector3d> bbEightCorners;
	bbEightCorners.emplace_back(Vector3d(BB.min.x, BB.min.y, BB.min.z));
	bbEightCorners.emplace_back(Vector3d(BB.min.x, BB.min.y, BB.max.z));
	bbEightCorners.emplace_back(Vector3d(BB.min.x, BB.max.y, BB.min.z));
	bbEightCorners.emplace_back(Vector3d(BB.min.x, BB.max.y, BB.max.z));
	bbEightCorners.emplace_back(Vector3d(BB.max.x, BB.min.y, BB.min.z));
	bbEightCorners.emplace_back(Vector3d(BB.max.x, BB.min.y, BB.max.z));
	bbEightCorners.emplace_back(Vector3d(BB.max.x, BB.max.y, BB.min.z));
	bbEightCorners.emplace_back(Vector3d(BB.max.x, BB.max.y, BB.max.z));
	
	std::vector<double> distancesFromPlane;
	for (const auto& corner : bbEightCorners) {
		distancesFromPlane.push_back(-(p.a * corner.x + p.b * corner.y + p.c * corner.z));
	}

	auto minElement = std::min_element(distancesFromPlane.begin(), distancesFromPlane.end());
	auto maxElement = std::max_element(distancesFromPlane.begin(), distancesFromPlane.end());
	this->Dmin = *minElement;
	this->Dmax = *maxElement;

	//set scrollbar min,max and pos
	double D_range = Dmax - Dmin;
	double offset = p.d - Dmin;
	Saturate(offset, 0.0, D_range);

	dScrollBar->SetPosition(static_cast<int>(1000.0*offset/D_range));
	dScrollBar->SetEnabled(true);

	DminLabel->SetText(fmt::format("D_min: {:.6g}", Dmin));
	DmaxLabel->SetText(fmt::format("D_max: {:.6g}", Dmax));
}

void CrossSection::FillTextboxValues(const Plane& P) {
	aTextbox->SetText(P.a,7);
	bTextbox->SetText(P.b,7);
	cTextbox->SetText(P.c,7);
	dTextbox->SetText(P.d,7);
}

Plane CrossSection::ReadTextboxValues() {
	Plane result;
	if (!(aTextbox->GetNumber(&result.a))) {
		throw Error("Invalid A coefficient");
	}
	if (!(bTextbox->GetNumber(&result.b))) {
		throw Error("Invalid B coefficient");
	}
	if (!(cTextbox->GetNumber(&result.c))) {
		throw Error("Invalid C coefficient");
	}
	if (!(dTextbox->GetNumber(&result.d))) {
		throw Error("Invalid D coefficient");
	}
	if ((result.a == 0.0) && (result.b == 0.0) && (result.c == 0.0)) {
		throw Error("A, B, C are all zero. That's not a valid plane.");
	}
	return result;
}