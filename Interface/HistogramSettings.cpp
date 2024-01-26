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
#include "HistogramSettings.h"
#include "HistogramPlotter.h" //To call refresh

#include "GLApp/GLToggle.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "Helper/MathTools.h" //Contains()

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

HistogramSettings::HistogramSettings(InterfaceGeometry *g, Worker *w):GLWindow() {

	int wD = 270;
	int panelHeight = 240;
	int hD = 2*panelHeight+10;

#if defined(MOLFLOW)
	hD += 6 * 25; //Time parameters
	panelHeight += 50;
#endif

	SetTitle("Histogram settings");

	//Global histogram settings
	globalSettingsPanel = new GLTitledPanel("Global histogram");
	globalSettingsPanel->SetBounds(5, 5, wD - 11, panelHeight);
	Add(globalSettingsPanel);

	globalRecordBounceToggle = new GLToggle(0, "Record bounces until absorbtion");
	globalSettingsPanel->SetCompBounds(globalRecordBounceToggle, 5, 15, 175, 25);
	globalSettingsPanel->Add(globalRecordBounceToggle);
	globalToggles.push_back(globalRecordBounceToggle);

	GLLabel* globalLabel1 = new GLLabel("Max recorded no. of bounces:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordBounceToggle, globalLabel1, 0, 25, globalRecordBounceToggle->GetWidth(), 20);
	globalSettingsPanel->Add(globalLabel1);

	globalHitLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel1, globalHitLimitText, globalLabel1->GetWidth()+5, 0, 70, globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalHitLimitText);
	globalTextFields.push_back(globalHitLimitText);

	GLLabel* globalLabel2 = new GLLabel("Bounces bin size:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel1, globalLabel2, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel2);

	globalHitBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitLimitText, globalHitBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalHitBinsizeText);
	globalTextFields.push_back(globalHitBinsizeText);

	globalRecordDistanceToggle = new GLToggle(0, "Record flight distance until absorption");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel2, globalRecordDistanceToggle, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalRecordDistanceToggle);
	globalToggles.push_back(globalRecordDistanceToggle);
	
	GLLabel* globalLabel3 = new GLLabel("Max recorded flight distance (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordDistanceToggle, globalLabel3, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel3);

	globalDistanceLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitBinsizeText, globalDistanceLimitText, 0, 50, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalDistanceLimitText);
	globalTextFields.push_back(globalDistanceLimitText);

	GLLabel* globalLabel4 = new GLLabel("Distance bin size (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel3, globalLabel4, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel4);

	globalDistanceBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceLimitText, globalDistanceBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalDistanceBinsizeText);
	globalTextFields.push_back(globalDistanceBinsizeText);

#if defined(MOLFLOW)
	globalRecordTimeToggle = new GLToggle(0, "Record flight time until absorption");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel4, globalRecordTimeToggle, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalRecordTimeToggle);
	globalToggles.push_back(globalRecordTimeToggle);

	GLLabel* globalLabel5 = new GLLabel("Max recorded flight time (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordTimeToggle, globalLabel5, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel5);

	globalTimeLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceBinsizeText, globalTimeLimitText, 0, 50, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalTimeLimitText);
	globalTextFields.push_back(globalTimeLimitText);

	GLLabel* globalLabel6 = new GLLabel("Time bin size (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel5, globalLabel6, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel6);

	globalTimeBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalTimeLimitText, globalTimeBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalTimeBinsizeText);
	globalTextFields.push_back(globalTimeBinsizeText);

	globalMemoryEstimateLabel_current = new GLLabel("Current memory (global):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel6, globalMemoryEstimateLabel_current, 0, 25, globalLabel3->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalMemoryEstimateLabel_current);

	globalMemoryEstimateLabel_new = new GLLabel("After applying (global):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalMemoryEstimateLabel_current, globalMemoryEstimateLabel_new, 0, 25, globalLabel3->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalMemoryEstimateLabel_new);
#endif


	//Facet histogram settings
	facetSettingsPanel = new GLTitledPanel("Facet histogram");
	SetCompBoundsRelativeTo(globalSettingsPanel, facetSettingsPanel, 0, globalSettingsPanel->GetHeight() + 5, globalSettingsPanel->GetWidth(), globalSettingsPanel->GetHeight());
	Add(facetSettingsPanel);

	facetRecordBounceToggle = new GLToggle(0, "Record bounces until absorbtion");
	facetSettingsPanel->SetCompBounds(facetRecordBounceToggle, 5, 15, 175, 25);
	facetSettingsPanel->Add(facetRecordBounceToggle);
	facetToggles.push_back(facetRecordBounceToggle);

	GLLabel* facetLabel1 = new GLLabel("Max recorded no. of bounces:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordBounceToggle, facetLabel1, 0, 25, facetRecordBounceToggle->GetWidth(), 20);
	facetSettingsPanel->Add(facetLabel1);

	facetHitLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel1, facetHitLimitText, facetLabel1->GetWidth() + 5, 0, 70, facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetHitLimitText);
	facetTextFields.push_back(facetHitLimitText);

	GLLabel* facetLabel2 = new GLLabel("Bounces bin size:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel1, facetLabel2, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel2);

	facetHitBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitLimitText, facetHitBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetHitBinsizeText);
	facetTextFields.push_back(facetHitBinsizeText);

	facetRecordDistanceToggle = new GLToggle(0, "Record flight distance until absorption");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel2, facetRecordDistanceToggle, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetRecordDistanceToggle);
	facetToggles.push_back(facetRecordDistanceToggle);

	GLLabel* facetLabel3 = new GLLabel("Max recorded flight distance (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordDistanceToggle, facetLabel3, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel3);

	facetDistanceLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitBinsizeText, facetDistanceLimitText, 0, 50, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetDistanceLimitText);
	facetTextFields.push_back(facetDistanceLimitText);

	GLLabel* facetLabel4 = new GLLabel("Distance bin size (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel3, facetLabel4, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel4);

	facetDistanceBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceLimitText, facetDistanceBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetDistanceBinsizeText);
	facetTextFields.push_back(facetDistanceBinsizeText);

#if defined(MOLFLOW)
	facetRecordTimeToggle = new GLToggle(0, "Record flight time until absorption");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel4, facetRecordTimeToggle, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetRecordTimeToggle);
	facetToggles.push_back(facetRecordTimeToggle);

	GLLabel* facetLabel5 = new GLLabel("Max recorded flight time (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordTimeToggle, facetLabel5, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel5);

	facetTimeLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceBinsizeText, facetTimeLimitText, 0, 50, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetTimeLimitText);
	facetTextFields.push_back(facetTimeLimitText);

	GLLabel* facetLabel6 = new GLLabel("Time bin size (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel5, facetLabel6, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel6);

	facetTimeBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetTimeLimitText, facetTimeBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetTimeBinsizeText);
	facetTextFields.push_back(facetTimeBinsizeText);
#endif

	facetMemoryEstimateLabel_current = new GLLabel("Current memory (all facets):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel6, facetMemoryEstimateLabel_current, 0, 25, facetLabel3->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetMemoryEstimateLabel_current);

	facetMemoryEstimateLabel_new = new GLLabel("After applying (sel.facets):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetMemoryEstimateLabel_current, facetMemoryEstimateLabel_new, 0, 25, facetLabel3->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetMemoryEstimateLabel_new);

	applyButton = new GLButton(0,"Apply");
	applyButton->SetBounds(wD/2-50,hD-44,100,21);
	Add(applyButton);
	

	// Right center
	SetBounds(5,35,wD,hD); //Default position

	RestoreDeviceObjects();
	
	this->interfGeom = g;
	this->work = w;
}

void HistogramSettings::Apply() {
	//Check input, return false if error, otherwise apply and return true
	HistogramGUISettings guiSettings;
	try {
		 guiSettings = GetGUIValues();
	}
	catch (const Error& err) {
		GLMessageBox::Display(err.what(), "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
		return;
	}	

	if (mApp->AskToReset()) {
		//Apply
		work->model->sp.globalHistogramParams = guiSettings.globalParams; //no mixed state, apply all

		auto selectedFacets = interfGeom->GetSelectedFacets();
		for (const auto facetId : selectedFacets) {
			InterfaceFacet* f = interfGeom->GetFacet(facetId);
			if (!guiSettings.facetRecBounceMixed) f->sh.facetHistogramParams.recordBounce = guiSettings.facetParams.recordBounce;
			if (!guiSettings.facetHitLimitMixed) f->sh.facetHistogramParams.nbBounceMax = guiSettings.facetParams.nbBounceMax;
			if (!guiSettings.facetHitBinsizeMixed) f->sh.facetHistogramParams.nbBounceBinsize = guiSettings.facetParams.nbBounceBinsize;

			if (!guiSettings.facetRecDistanceMixed) f->sh.facetHistogramParams.recordDistance = guiSettings.facetParams.recordDistance;
			if (!guiSettings.facetDistanceLimitMixed) f->sh.facetHistogramParams.distanceMax = guiSettings.facetParams.distanceMax;
			if (!guiSettings.facetDistanceBinsizeMixed) f->sh.facetHistogramParams.distanceBinsize = guiSettings.facetParams.distanceBinsize;
#if defined(MOLFLOW)
			if (!guiSettings.facetRecTimeMixed) f->sh.facetHistogramParams.recordTime = guiSettings.facetParams.recordTime;
			if (!guiSettings.facetTimeLimitMixed) f->sh.facetHistogramParams.timeMax = guiSettings.facetParams.timeMax;
			if (!guiSettings.facetTimeBinsizeMixed) f->sh.facetHistogramParams.timeBinsize = guiSettings.facetParams.timeBinsize;
#endif
		}

		mApp->changedSinceSave = true;
		work->needsReload = true; // to trigger realreload in update
		try{
		    work->Update(mApp->m_fTime); //To refresh histogram cache
        }
        catch (const std::exception &e) {
            GLMessageBox::Display(e.what(), "Histogram Apply Error", GLDLG_OK, GLDLG_ICONERROR);
        }
		if (mApp->histogramPlotter) mApp->histogramPlotter->Refresh();
		Refresh(interfGeom->GetSelectedFacets());
	}
}

void HistogramSettings::Refresh(const std::vector<size_t>& selectedFacetIds) {
	
	//Global histogram panel
	globalRecordBounceToggle->SetState(work->model->sp.globalHistogramParams.recordBounce);
	globalHitLimitText->SetText(work->model->sp.globalHistogramParams.nbBounceMax);
	globalHitBinsizeText->SetText(work->model->sp.globalHistogramParams.nbBounceBinsize);
	globalRecordDistanceToggle->SetState(work->model->sp.globalHistogramParams.recordDistance);
	globalDistanceLimitText->SetText(work->model->sp.globalHistogramParams.distanceMax);
	globalDistanceBinsizeText->SetText(work->model->sp.globalHistogramParams.distanceBinsize);
#if defined(MOLFLOW)
	globalRecordTimeToggle->SetState(work->model->sp.globalHistogramParams.recordTime);
	globalTimeLimitText->SetText(work->model->sp.globalHistogramParams.timeMax);
	globalTimeBinsizeText->SetText(work->model->sp.globalHistogramParams.timeBinsize);
#endif
	UpdateMemoryEstimate_New_Global();

	//Facet histogram panel
	nbSelectedFacetCache = selectedFacetIds.size();
	bool hasFacetSelected = selectedFacetIds.size() > 0;
	if (!hasFacetSelected) {
		facetRecordBounceToggle->SetState(false);
		facetHitLimitText->SetText("");
		facetHitBinsizeText->SetText("");
		facetRecordDistanceToggle->SetState(false);
		facetDistanceLimitText->SetText("");
		facetDistanceBinsizeText->SetText("");
#if defined(MOLFLOW)
		facetRecordTimeToggle->SetState(false);
		facetTimeLimitText->SetText("");
		facetTimeBinsizeText->SetText("");
#endif
	}
	facetRecordBounceToggle->SetEnabled(hasFacetSelected);
	facetRecordDistanceToggle->SetEnabled(hasFacetSelected);
#if defined(MOLFLOW)
    facetRecordTimeToggle->SetEnabled(hasFacetSelected);
#endif

    if (hasFacetSelected) {
		//Fill in facet-specific text
		bool recordBounceEqual = true, bounceMaxEqual = true, bounceBinsizeEqual = true,
			recordDistanceEqual = true, distanceMaxEqual = true, distanceBinsizeEqual = true;
#if defined(MOLFLOW)
		bool recordTimeEqual = true, timeMaxEqual = true, timeBinsizeEqual = true;
#endif
		InterfaceFacet* f0 = interfGeom->GetFacet(selectedFacetIds[0]);
		bool recBounce = f0->sh.facetHistogramParams.recordBounce;
		size_t bounceMax = f0->sh.facetHistogramParams.nbBounceMax;
		size_t bounceBinsize = f0->sh.facetHistogramParams.nbBounceBinsize;
		bool recDist = f0->sh.facetHistogramParams.recordDistance;
		double distMax = f0->sh.facetHistogramParams.distanceMax;
		double distBinsize = f0->sh.facetHistogramParams.distanceBinsize;
#if defined(MOLFLOW)
		bool recTime = f0->sh.facetHistogramParams.recordTime;
		double timeMax = f0->sh.facetHistogramParams.timeMax;
		double timeBinsize = f0->sh.facetHistogramParams.timeBinsize;
#endif

		for (size_t i = 1; i < selectedFacetIds.size();i++) {
			InterfaceFacet* f = interfGeom->GetFacet(selectedFacetIds[i]);
			recordBounceEqual = recordBounceEqual && (f->sh.facetHistogramParams.recordBounce == recBounce);
			bounceMaxEqual = bounceMaxEqual && (f->sh.facetHistogramParams.nbBounceMax == bounceMax);
			bounceBinsizeEqual = bounceBinsizeEqual && (f->sh.facetHistogramParams.nbBounceBinsize== bounceBinsize);
			recordDistanceEqual = recordDistanceEqual && (f->sh.facetHistogramParams.recordDistance == recDist);
			distanceMaxEqual = distanceMaxEqual && (f->sh.facetHistogramParams.distanceMax == distMax);
			distanceBinsizeEqual = distanceBinsizeEqual && (f->sh.facetHistogramParams.distanceBinsize == distBinsize);
#if defined(MOLFLOW)
			recordTimeEqual = recordDistanceEqual && (f->sh.facetHistogramParams.recordDistance == recDist);
			timeMaxEqual = timeMaxEqual && (f->sh.facetHistogramParams.timeMax == timeMax);
			timeBinsizeEqual = timeBinsizeEqual && (f->sh.facetHistogramParams.timeBinsize == timeBinsize);
#endif
		}

		facetRecordBounceToggle->AllowMixedState(!recordBounceEqual);
		facetRecordBounceToggle->SetState(recordBounceEqual ? recBounce : 2);
		if (bounceMaxEqual) {
			facetHitLimitText->SetText(bounceMax);
		}
		else {
			facetHitLimitText->SetText("...");
		}
		if (bounceBinsizeEqual) {
			facetHitBinsizeText->SetText(bounceBinsize);
		}
		else {
			facetHitBinsizeText->SetText("...");
		}

		facetRecordDistanceToggle->AllowMixedState(!recordDistanceEqual);
		facetRecordDistanceToggle->SetState(recordDistanceEqual ? recDist : 2);
		if (distanceMaxEqual) {
			facetDistanceLimitText->SetText(distMax);
		}
		else {
			facetDistanceLimitText->SetText("...");
		}
		if (distanceBinsizeEqual) {
			facetDistanceBinsizeText->SetText(distBinsize);
		}
		else {
			facetDistanceBinsizeText->SetText("...");
		}
#if defined(MOLFLOW)
		facetRecordTimeToggle->AllowMixedState(!recordTimeEqual);
		facetRecordTimeToggle->SetState(recordTimeEqual ? recTime : 2);
		if (timeMaxEqual) {
			facetTimeLimitText->SetText(timeMax);
		}
		else {
			facetTimeLimitText->SetText("...");
		}
		if (timeBinsizeEqual) {
			facetTimeBinsizeText->SetText(timeBinsize);
		}
		else {
			facetTimeBinsizeText->SetText("...");
		}
#endif

	}
	UpdateMemoryEstimate_New_SelectedFacets(nbSelectedFacetCache);
	UpdateMemoryEstimate_Current();
	EnableDisableControls();
}

void HistogramSettings::EnableDisableControls() {
	//Global
	globalHitLimitText->SetEditable(globalRecordBounceToggle->GetState());
	globalHitBinsizeText->SetEditable(globalRecordBounceToggle->GetState());
	globalDistanceLimitText->SetEditable(globalRecordDistanceToggle->GetState());
	globalDistanceBinsizeText->SetEditable(globalRecordDistanceToggle->GetState());
#if defined(MOLFLOW)
	globalTimeLimitText->SetEditable(globalRecordTimeToggle->GetState());
	globalTimeBinsizeText->SetEditable(globalRecordTimeToggle->GetState());
#endif

	//Selected facets (mixed enabled state: don't allow value edit)
	facetHitLimitText->SetEditable(facetRecordBounceToggle->GetState()==1);
	facetHitBinsizeText->SetEditable(facetRecordBounceToggle->GetState()==1);
	facetDistanceLimitText->SetEditable(facetRecordDistanceToggle->GetState()==1);
	facetDistanceBinsizeText->SetEditable(facetRecordDistanceToggle->GetState()==1);
#if defined(MOLFLOW)
	facetTimeLimitText->SetEditable(facetRecordTimeToggle->GetState()==1);
	facetTimeBinsizeText->SetEditable(facetRecordTimeToggle->GetState()==1);
#endif
}

void HistogramSettings::ProcessMessage(GLComponent *src,int message) {
	

	switch (message) {
		
		case MSG_BUTTON:

			if (src==applyButton) {
				//Set histogram parameters on selected facets
				Apply();
			}
			break;
		
		case MSG_TOGGLE:
			EnableDisableControls();
			
			if (Contains(globalToggles, src)) {
				UpdateMemoryEstimate_New_Global();
			}
			else if (Contains(facetToggles, src)) {
				UpdateMemoryEstimate_New_SelectedFacets(nbSelectedFacetCache);
			}
			break;
		case MSG_TEXT_UPD: //Update estimate as you type
			if (Contains(globalTextFields, src)) {
				UpdateMemoryEstimate_New_Global();
			}
			else if (Contains(facetTextFields, src)) {
				UpdateMemoryEstimate_New_SelectedFacets(nbSelectedFacetCache);
			}
			break;		
	}
	GLWindow::ProcessMessage(src,message);
}

HistogramSettings::HistogramGUISettings HistogramSettings::GetGUIValues()
{
	HistogramGUISettings result;

	//Global histogram - no mixed state
	result.globalParams.recordBounce = globalRecordBounceToggle->GetState();

	if (result.globalParams.recordBounce) {

		if (globalHitLimitText->GetText() != "...")
		{
			if (!globalHitLimitText->GetNumberSizeT(&result.globalParams.nbBounceMax) || result.globalParams.nbBounceMax <= 0) {
				throw Error("Global bounce limit must be a positive integer");
			}
		}

		if (globalHitBinsizeText->GetText() != "...")
		{
			if (!globalHitBinsizeText->GetNumberSizeT(&result.globalParams.nbBounceBinsize) || result.globalParams.nbBounceBinsize <= 0) {
				throw Error("Global bounce bin size must be a positive integer");
			}
		}
	}

	result.globalParams.recordDistance = globalRecordDistanceToggle->GetState();

	if (result.globalParams.recordDistance) {
		if (globalDistanceLimitText->GetText() != "...") {
			if (!globalDistanceLimitText->GetNumber(&result.globalParams.distanceMax) || result.globalParams.distanceMax <= 0.0) {
				throw Error("Global distance limit must be a positive scalar");
			}
		}

		if (globalDistanceBinsizeText->GetText() != "...") {

			if (!globalDistanceBinsizeText->GetNumber(&result.globalParams.distanceBinsize) || result.globalParams.distanceBinsize <= 0) {
				throw Error("Global distance bin size must be a positive scalar");
			}
		}
	}

#if defined(MOLFLOW)
	result.globalParams.recordTime = globalRecordTimeToggle->GetState();

	if (result.globalParams.recordTime) {
		if (globalTimeLimitText->GetText() != "...") {

			if (!globalTimeLimitText->GetNumber(&result.globalParams.timeMax) || result.globalParams.timeMax <= 0) {
				throw Error("Global time limit must be a positive scalar");
			}
		}

		if (globalTimeBinsizeText->GetText() != "...") {

			if (!globalTimeBinsizeText->GetNumber(&result.globalParams.timeBinsize) || result.globalParams.timeBinsize <= 0) {
				throw Error("Global time bin size must be a positive scalar");
			}
		}
	}
#endif



	//Selected facets - each setting can have mixed state
	//For code compacting, the if conditions also assign values, might be hard to read

	if (!(result.facetRecBounceMixed = facetRecordBounceToggle->GetState() == 2)) { //ignore values otherwise, since they won't be applied
		if (result.facetParams.recordBounce = (bool)facetRecordBounceToggle->GetState()) { //ignore values otherwise, since histogram rec. disabled
			if (!(result.facetHitLimitMixed = (facetHitLimitText->GetText() == "..."))) { //ignore value otherwise, as mixed
				if (!facetHitLimitText->GetNumberSizeT(&result.facetParams.nbBounceMax) || result.facetParams.nbBounceMax <= 0) {
					throw Error("Facet bounce limit must be a positive integer");
				}
			} //if not mixed value

			if (!(result.facetHitBinsizeMixed = (facetHitBinsizeText->GetText() == "...")))	{
				if (!facetHitBinsizeText->GetNumberSizeT(&result.facetParams.nbBounceBinsize) || result.facetParams.nbBounceBinsize <= 0) {
					throw Error("Facet bounce bin size must be a positive integer");
				}
			} //if not mixed value
		} //if bounce recording on
	} //if not mixed state

	if (!(result.facetRecDistanceMixed = facetRecordDistanceToggle->GetState() == 2)) { //ignore values otherwise, since they won't be applied
		if (result.facetParams.recordDistance = (bool)facetRecordDistanceToggle->GetState()) { //ignore values otherwise, since histogram rec. disabled
			if (!(result.facetDistanceLimitMixed = (facetDistanceLimitText->GetText() == "..."))) { //ignore value otherwise, as mixed
				if (!facetDistanceLimitText->GetNumber(&result.facetParams.distanceMax) || result.facetParams.distanceMax <= 0) {
					throw Error("Facet distance limit must be a positive number");
				}
			} //if not mixed value

			if (!(result.facetDistanceBinsizeMixed = (facetDistanceBinsizeText->GetText() == "..."))) {
				if (!facetDistanceBinsizeText->GetNumber(&result.facetParams.distanceBinsize) || result.facetParams.distanceBinsize <= 0) {
					throw Error("Facet distance bin size must be a positive number");
				}
			} //if not mixed value
		} //if bounce recording on
	} //if not mixed state

#if defined(MOLFLOW)
	if (!(result.facetRecTimeMixed = facetRecordTimeToggle->GetState() == 2)) { //ignore values otherwise, since they won't be applied
		if (result.facetParams.recordTime = (bool)facetRecordTimeToggle->GetState()) { //ignore values otherwise, since histogram rec. disabled
			if (!(result.facetTimeLimitMixed = (facetTimeLimitText->GetText() == "..."))) { //ignore value otherwise, as mixed
				if (!facetTimeLimitText->GetNumber(&result.facetParams.timeMax) || result.facetParams.timeMax <= 0) {
					throw Error("Facet time limit must be a positive number");
				}
			} //if not mixed value

			if (!(result.facetTimeBinsizeMixed = (facetTimeBinsizeText->GetText() == "..."))) {
				if (!facetTimeBinsizeText->GetNumber(&result.facetParams.timeBinsize) || result.facetParams.timeBinsize <= 0) {
					throw Error("Facet time bin size must be a positive number");
				}
			} //if not mixed value
		} //if bounce recording on
	} //if not mixed state
#endif
	return result;
}

void HistogramSettings::UpdateMemoryEstimate_New_Global() {
	HistogramGUISettings guiSettings;
	try {
		guiSettings = GetGUIValues();
		size_t memory_bytes = guiSettings.globalParams.GetDataSize() * (work->interfaceMomentCache.size()+1);
		globalMemoryEstimateLabel_new->SetText(fmt::format("After applying: {}", FormatMemory(memory_bytes)));
	}
	catch (...) {
		globalMemoryEstimateLabel_new->SetText("After applying: [invalid textbox value(s)]");
	}
}

void HistogramSettings::UpdateMemoryEstimate_New_SelectedFacets(size_t nbSelectedFacets) {
	HistogramGUISettings guiSettings;
	try {
		guiSettings = GetGUIValues();
		if ((guiSettings.facetRecBounceMixed //Mixed state
			|| (guiSettings.facetParams.recordBounce && //Recording enabled and a textbox in mixed state
				(guiSettings.facetHitLimitMixed	|| guiSettings.facetHitBinsizeMixed)))

			|| (guiSettings.facetRecDistanceMixed //Mixed state
				|| (guiSettings.facetParams.recordDistance && //Recording enabled and a textbox in mixed state
					(guiSettings.facetDistanceLimitMixed || guiSettings.facetDistanceBinsizeMixed)))

#if defined(MOLFLOW)
			|| (guiSettings.facetRecTimeMixed //Mixed state
				|| (guiSettings.facetParams.recordTime && //Recording enabled and a textbox in mixed state
					(guiSettings.facetTimeLimitMixed || guiSettings.facetTimeBinsizeMixed)))
#endif
			) {
			facetMemoryEstimateLabel_new->SetText("After applying (sel.facets): [mixed state]");
		}
		else {
			size_t memory_bytes = guiSettings.facetParams.GetDataSize() * nbSelectedFacets * (work->interfaceMomentCache.size() + 1);
			facetMemoryEstimateLabel_new->SetText(fmt::format("After applying (sel.facets): {}", FormatMemory(memory_bytes)));
		}
	}
	catch (...) {
		facetMemoryEstimateLabel_new->SetText("After applying (sel.facets): [invalid textbox value(s)]");
	}
}

void HistogramSettings::UpdateMemoryEstimate_Current()
{
	globalMemoryEstimateLabel_current->SetText(fmt::format("Current memory (global): {}",
		FormatMemory(work->model->sp.globalHistogramParams.GetDataSize() * (work->interfaceMomentCache.size() + 1))));
	size_t facetsHistogramSize = 0;
	size_t nbFacet = interfGeom->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		facetsHistogramSize += interfGeom->GetFacet(i)->sh.facetHistogramParams.GetDataSize();
	}
	facetsHistogramSize *= (work->interfaceMomentCache.size() + 1);
	facetMemoryEstimateLabel_current->SetText(fmt::format("Current memory (all facets): {}", FormatMemory(facetsHistogramSize)));
}