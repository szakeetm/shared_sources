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
#include "SelectFacetByResult.h"
#include "GLApp/GLMessageBox.h"
#include "Facet_shared.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
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
SelectFacetByResult::SelectFacetByResult(Worker *w) :GLWindow() {

	int xD, yD, wD;
	wD = 250;
	SetTitle("Select facets by simulation result");

	int startX = 5;
	int startY = 10;

	GLLabel* infoLabel = new GLLabel("Empty textbox = condition ignored.");
	infoLabel->SetBounds(5,5,wD-10,20);
	Add(infoLabel);
	
	startX = 5;
	startY+=25;
	hitsMoreThanText = new GLTextField(0,"");
	hitsMoreThanText->SetBounds(startX,startY,80,20);
	Add(hitsMoreThanText);
	startX+=90;
	GLLabel* hitsLabel = new GLLabel(" < Hits < ");
	hitsLabel->SetBounds(startX,startY,60,20);
	Add(hitsLabel);
	startX+=70;
	
	hitsLessThanText = new GLTextField(0,"");
	hitsLessThanText->SetBounds(startX,startY,80,20);
	Add(hitsLessThanText);
	




	
	startX=5;
	startY+=25;



	absMoreThanText = new GLTextField(0,"");
	absMoreThanText->SetBounds(startX,startY,80,20);
	Add(absMoreThanText);
	startX+=90;
	GLLabel* absLabel = new GLLabel(" < Abs < ");
	absLabel->SetBounds(startX,startY,60,20);
	Add(absLabel);
	startX+=70;

	absLessThanText = new GLTextField(0,"");
	absLessThanText->SetBounds(startX,startY,80,20);
	Add(absLessThanText);
	

	
	startX=5;
	startY+=25;

	#ifdef MOLFLOW
	
	desMoreThanText = new GLTextField(0,"");
	desMoreThanText->SetBounds(startX,startY,80,20);
	Add(desMoreThanText);
	startX+=90;
	GLLabel* desLabel = new GLLabel(" < Des < ");
	desLabel->SetBounds(startX,startY,60,20);
	Add(desLabel);
	startX+=70;

	desLessThanText = new GLTextField(0,"");
	desLessThanText->SetBounds(startX,startY,80,20);
	Add(desLessThanText);
	
	
	startX=5;
	startY+=25;
	#endif

	#ifdef SYNRAD

	fluxMoreThanText = new GLTextField(0,"");
	fluxMoreThanText->SetBounds(startX,startY,80,20);
	Add(fluxMoreThanText);
	startX+=90;
	GLLabel* fluxLabel = new GLLabel(" < Flux < ");
	fluxLabel->SetBounds(startX,startY,60,20);
	Add(fluxLabel);
	startX+=70;

	fluxLessThanText = new GLTextField(0,"");
	fluxLessThanText->SetBounds(startX,startY,80,20);
	Add(fluxLessThanText);
	
	
	startX=5;
	startY+=25;


	powerMoreThanText = new GLTextField(0,"");
	powerMoreThanText->SetBounds(startX,startY,80,20);
	Add(powerMoreThanText);
	startX+=90;
	GLLabel* powerLabel = new GLLabel(" < Power < ");
	powerLabel->SetBounds(startX,startY,60,20);
	Add(powerLabel);
	startX+=70;

	powerLessThanText = new GLTextField(0,"");
	powerLessThanText->SetBounds(startX,startY,80,20);
	Add(powerLessThanText);
	

	
	startX=5;
	startY+=25;
	#endif

	selectButton = new GLButton(0, "Select");
	selectButton->SetBounds(startX, startY, 65, 20);
	Add(selectButton);
	
	startX += 70;
	addSelectButton = new GLButton(0, "Add to sel.");
	addSelectButton->SetBounds(startX, startY, 70, 20);
	Add(addSelectButton);
	startX += 75;
remSelectButton = new GLButton(GLDLG_SELECT_REM, "Remove from sel.");
	remSelectButton->SetBounds(startX, startY, 95, 20);
	Add(remSelectButton);
	
	
	startY+=25;

	// Top left
	SetBounds(10, 30, wD, startY+30);

	// Create objects
	RestoreDeviceObjects();

	work = w;
	geom = w->GetGeometry();
}

void SelectFacetByResult::ProcessMessage(GLComponent *src, int message) {
	
	if (message == MSG_BUTTON) {
		bool do_hitLess,do_hitMore; double hitLess,hitMore;
		bool do_absLess,do_absMore; double absLess,absMore;
		#ifdef MOLFLOW
		bool do_desLess,do_desMore; double desLess,desMore;
		#endif
		#ifdef SYNRAD
		bool do_fluxLess,do_fluxMore; double fluxLess,fluxMore;
		bool do_powerLess,do_powerMore; double powerLess,powerMore;
		#endif

		if (hitsMoreThanText->GetText().empty()) {
			do_hitMore = false;
		} else {
			if (!hitsMoreThanText->GetNumber(&hitMore)) {
				GLMessageBox::Display("Hits more than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_hitMore = true;
		}
		if (hitsLessThanText->GetText().empty()) {
			do_hitLess = false;
		} else {
			if (!hitsLessThanText->GetNumber(&hitLess)) {
				GLMessageBox::Display("Hits less than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_hitLess = true;
		}

		if (absMoreThanText->GetText().empty()) {
			do_absMore = false;
		} else {
			if (!absMoreThanText->GetNumber(&absMore)) {
				GLMessageBox::Display("abs more than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_absMore = true;
		}
		if (absLessThanText->GetText().empty()) {
			do_absLess = false;
		} else {
			if (!absLessThanText->GetNumber(&absLess)) {
				GLMessageBox::Display("abs less than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_absLess = true;
		}

		#ifdef MOLFLOW
				if (desMoreThanText->GetText().empty()) {
			do_desMore = false;
		} else {
			if (!desMoreThanText->GetNumber(&desMore)) {
				GLMessageBox::Display("des more than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_desMore = true;
		}
		if (desLessThanText->GetText().empty()) {
			do_desLess = false;
		} else {
			if (!desLessThanText->GetNumber(&desLess)) {
				GLMessageBox::Display("des less than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_desLess = true;
		}
		#endif

		#ifdef SYNRAD
		if (fluxMoreThanText->GetText().empty()) {
			do_fluxMore = false;
		} else {
			if (!fluxMoreThanText->GetNumber(&fluxMore)) {
				GLMessageBox::Display("flux more than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_fluxMore = true;
		}
		if (fluxLessThanText->GetText().empty()) {
			do_fluxLess = false;
		} else {
			if (!fluxLessThanText->GetNumber(&fluxLess)) {
				GLMessageBox::Display("flux less than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_fluxLess = true;
		}

						if (powerMoreThanText->GetText().empty()) {
			do_powerMore = false;
		} else {
			if (!powerMoreThanText->GetNumber(&powerMore)) {
				GLMessageBox::Display("power more than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_powerMore = true;
		}
		if (powerLessThanText->GetText().empty()) {
			do_powerLess = false;
		} else {
			if (!powerLessThanText->GetNumber(&powerLess)) {
				GLMessageBox::Display("power less than number invalid>", "Error", GLDLG_OK, GLDLG_ICONINFO);
				return;
			}
			do_powerLess = true;
		}
		#endif

		if (src==selectButton) geom->UnselectAll();
		//Form valid, let's do the work
		size_t nbFacet = geom->GetNbFacet();
		for (size_t i=0;i<nbFacet;i++) {
			InterfaceFacet* f=geom->GetFacet(i);
			bool match=true;
			if (do_hitLess) match = match && (f->facetHitCache.nbMCHit<hitLess);
			if (do_hitMore) match = match && (f->facetHitCache.nbMCHit>hitMore);
			if (do_absLess) match = match && (f->facetHitCache.nbAbsEquiv<absLess);
			if (do_absMore) match = match && (f->facetHitCache.nbAbsEquiv>absMore);
			#ifdef MOLFLOW
			if (do_desLess) match = match && (f->facetHitCache.nbDesorbed<hitLess);
			if (do_desMore) match = match && (f->facetHitCache.nbDesorbed>hitMore);
			#endif
			#ifdef SYNRAD
			if (do_fluxLess) match = match && (f->facetHitCache.fluxAbs/work->no_scans < fluxLess);
			if (do_fluxMore) match = match && (f->facetHitCache.fluxAbs/work->no_scans > fluxMore);
			if (do_powerLess) match = match && (f->facetHitCache.powerAbs/work->no_scans < powerLess);
			if (do_powerMore) match = match && (f->facetHitCache.powerAbs/work->no_scans > powerMore);
			#endif
			if (match) f->selected = (src!=remSelectButton);
		}
		geom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected();

	}
	
	GLWindow::ProcessMessage(src, message);
	
}