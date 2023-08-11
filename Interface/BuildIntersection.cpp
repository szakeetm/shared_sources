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
#include "Geometry_shared.h"
#include "BuildIntersection.h"
#include "Facet_shared.h" //DeletedFacet
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
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

/**
* \brief Constructor with initialisation for the BuildIntersection window (Facet/Build intersection)
* \param g pointer to the InterfaceGeometry
* \param w Worker handle
*/
BuildIntersection::BuildIntersection(InterfaceGeometry *g, Worker *w) : GLWindow() {

	int wD = 330;
	int hD = 130;

    nbFacet = 0;
    nbCreated = 0;

	label1 = new GLLabel("Select all facets that form part of the intersection.\n"
		"Then select vertices that you'd like to ensure are being kept.\n"
		"Facets without any selected vertex won't be altered");
	label1->SetBounds(10, 10, 180, 40);
	Add(label1);

	resultLabel = new GLLabel("resultLabel");
	resultLabel->SetBounds(10, 90, 180, 20);
	Add(resultLabel);

	buildButton = new GLButton(0, "Build intersection");
	buildButton->SetBounds(10, 65, 100, 20);
	Add(buildButton);

	undoButton = new GLButton(0, "Undo");
	undoButton->SetBounds(120, 65, 100, 20);
	Add(undoButton);

	SetTitle("Build Intersection");
	// Center dialog
	//int wS, hS;
	//GLToolkit::GetScreenSize(&wS, &hS);
	//int xD = (wS - wD) / 2;
	//int yD = (hS - hD) / 2;
	//SetBounds(xD, yD, wD, hD);
	GLWindow::SetBounds(10, 30,wD,hD);
	GLContainer::RestoreDeviceObjects();

	resultLabel->SetText("");
	undoButton->SetEnabled(false);
	interfGeom = g;
	work = w;
}

/**
* \brief Destructor that will destroy old undo facets
*/
BuildIntersection::~BuildIntersection() {
	ClearUndoFacets();
}

/**
* \brief Destroy old undo facets
*/
void BuildIntersection::ClearUndoFacets() {
	//Destroy old undo facets
	for (DeletedFacet delFacet : deletedFacetList)
		SAFE_DELETE(delFacet.f);
	deletedFacetList.clear();
	resultLabel->SetText("");
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void BuildIntersection::ProcessMessage(GLComponent *src, int message) {

	if(message == MSG_BUTTON) {
		if (src == undoButton) {
			if (nbFacet == interfGeom->GetNbFacet()) { //Assume no change since the split operation
				std::vector<size_t> newlyCreatedList;
				for (size_t index = (interfGeom->GetNbFacet() - nbCreated);index < interfGeom->GetNbFacet();index++) {
					newlyCreatedList.push_back(index);
				}
				interfGeom->RemoveFacets(newlyCreatedList);
				interfGeom->RestoreFacets(deletedFacetList, false); //Restore to original position
			}
			else {
				int answer = GLMessageBox::Display("Geometry changed since intersecting, restore to end without deleting the newly created facets?", "Split undo", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO);
				if (answer == GLDLG_OK) interfGeom->RestoreFacets(deletedFacetList, true); //Restore to end
				else return;
			}
			deletedFacetList.clear();
			undoButton->SetEnabled(false);
			resultLabel->SetText("");
			//Renumberformula
			work->MarkToReload();
			mApp->UpdateModelParams();
			mApp->UpdateFacetlistSelected();
			mApp->UpdateViewers();
		}
		else if (src == buildButton) {
			if (interfGeom->GetNbSelectedFacets() < 2) {
			GLMessageBox::Display("Select at least 2 facets", "Can't create intersection", GLDLG_OK, GLDLG_ICONERROR);
			return;
			}
			if (mApp->AskToReset()) {
				ClearUndoFacets();
				nbCreated = 0;
				deletedFacetList = interfGeom->BuildIntersection(&nbCreated);
				nbFacet = interfGeom->GetNbFacet();
				std::stringstream tmp;
				tmp << deletedFacetList.size() << " facets intersected, creating " << nbCreated << " new.";
				resultLabel->SetText(tmp.str().c_str());
				if (!deletedFacetList.empty()) undoButton->SetEnabled(true);
				work->MarkToReload();
				mApp->UpdateModelParams();
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
			}
		}
	}

	GLWindow::ProcessMessage(src, message);
}