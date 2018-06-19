/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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

#include "GLApp\GLToggle.h"
#include "GLApp\GLTextField.h"
#include "GLApp\GLLabel.h"
#include "GLApp\GLTextField.h"
#include "GLApp\GLButton.h"
#include "GLApp\GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "Geometry_shared.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

HistogramSettings::HistogramSettings(Geometry *s, Worker *w):GLWindow() {

	int wD = 270;
	int panelHeight = 215;
	int hD = 2*panelHeight+85;

#ifdef MOLFLOW
	hD += 6 * 25; //Time parameters
	panelHeight += 50;
#endif

	SetTitle("Histogram settings");

	GLLabel *warningLabel = new GLLabel("Histograms only record absorption!");
	warningLabel->SetBounds(5, 5, wD - 11, 20);
	Add(warningLabel);

	//Global histogram settings
	globalSettingsPanel = new GLTitledPanel("Global histogram");
	globalSettingsPanel->SetBounds(5, 30, wD - 11, panelHeight);
	Add(globalSettingsPanel);

	globalRecordBounceToggle = new GLToggle(0, "Record bounces until absorbtion");
	globalSettingsPanel->SetCompBounds(globalRecordBounceToggle, 5, 15, 175, 25);
	globalSettingsPanel->Add(globalRecordBounceToggle);

	GLLabel* globalLabel1 = new GLLabel("Max recorded no. of bounces:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordBounceToggle, globalLabel1, 0, 25, globalRecordBounceToggle->GetWidth(), 20);
	globalSettingsPanel->Add(globalLabel1);

	globalHitLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel1, globalHitLimitText, globalLabel1->GetWidth()+5, 0, 70, globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalHitLimitText);

	GLLabel* globalLabel2 = new GLLabel("Bounces bin size:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel1, globalLabel2, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel2);

	globalHitBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitLimitText, globalHitBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalHitBinsizeText);

	globalRecordDistanceToggle = new GLToggle(0, "Record flight distance until absorbtion");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitBinsizeText, globalRecordDistanceToggle, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalRecordDistanceToggle);
	
	GLLabel* globalLabel3 = new GLLabel("Max recorded flight distance (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordDistanceToggle, globalLabel3, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel3);

	globalDistanceLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalHitBinsizeText, globalDistanceLimitText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalDistanceLimitText);

	GLLabel* globalLabel4 = new GLLabel("Distance bin size (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel3, globalLabel4, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel4);

	globalDistanceBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceLimitText, globalDistanceBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalDistanceBinsizeText);

#ifdef MOLFLOW
	globalRecordTimeToggle = new GLToggle(0, "Record flight time until absorbtion");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceBinsizeText, globalRecordTimeToggle, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalRecordTimeToggle);

	GLLabel* globalLabel5 = new GLLabel("Max recorded flight time (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordTimeToggle, globalLabel5, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel5);

	globalTimeLimitText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalDistanceBinsizeText, globalTimeLimitText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalTimeLimitText);

	GLLabel* globalLabel6 = new GLLabel("Time bin size (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel5, globalLabel6, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalLabel6);

	globalTimeBinsizeText = new GLTextField(0, "");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalTimeLimitText, globalTimeBinsizeText, 0, 25, globalHitLimitText->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalTimeBinsizeText);
#endif

	globalMemoryEstimateLabel = new GLLabel("Memory estimate of global histogram:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel6, globalMemoryEstimateLabel, 0, 25, globalLabel3->GetWidth(), globalLabel1->GetHeight());
	globalSettingsPanel->Add(globalMemoryEstimateLabel);


	//Facet histogram settings
	facetSettingsPanel = new GLTitledPanel("Facet histogram");
	facetSettingsPanel->SetBounds(5, 30, wD - 11, panelHeight);
	Add(facetSettingsPanel);

	facetRecordBounceToggle = new GLToggle(0, "Record bounces until absorbtion");
	facetSettingsPanel->SetCompBounds(facetRecordBounceToggle, 5, 15, 175, 25);
	facetSettingsPanel->Add(facetRecordBounceToggle);

	GLLabel* facetLabel1 = new GLLabel("Max recorded no. of bounces:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordBounceToggle, facetLabel1, 0, 25, facetRecordBounceToggle->GetWidth(), 20);
	facetSettingsPanel->Add(facetLabel1);

	facetHitLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel1, facetHitLimitText, facetLabel1->GetWidth() + 5, 0, 70, facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetHitLimitText);

	GLLabel* facetLabel2 = new GLLabel("Bounces bin size:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel1, facetLabel2, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel2);

	facetHitBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitLimitText, facetHitBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetHitBinsizeText);

	facetRecordDistanceToggle = new GLToggle(0, "Record flight distance until absorbtion");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitBinsizeText, facetRecordDistanceToggle, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetRecordDistanceToggle);

	GLLabel* facetLabel3 = new GLLabel("Max recorded flight distance (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordDistanceToggle, facetLabel3, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel3);

	facetDistanceLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetHitBinsizeText, facetDistanceLimitText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetDistanceLimitText);

	GLLabel* facetLabel4 = new GLLabel("Distance bin size (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel3, facetLabel4, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel4);

	facetDistanceBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceLimitText, facetDistanceBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetDistanceBinsizeText);

#ifdef MOLFLOW
	facetRecordTimeToggle = new GLToggle(0, "Record flight time until absorbtion");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceBinsizeText, facetRecordTimeToggle, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetRecordTimeToggle);

	GLLabel* facetLabel5 = new GLLabel("Max recorded flight time (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordTimeToggle, facetLabel5, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel5);

	facetTimeLimitText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetDistanceBinsizeText, facetTimeLimitText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetTimeLimitText);

	GLLabel* facetLabel6 = new GLLabel("Time bin size (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel5, facetLabel6, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetLabel6);

	facetTimeBinsizeText = new GLTextField(0, "");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetTimeLimitText, facetTimeBinsizeText, 0, 25, facetHitLimitText->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetTimeBinsizeText);
#endif

	facetMemoryEstimateLabel = new GLLabel("Memory estimate of facet histogram:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel6, facetMemoryEstimateLabel, 0, 25, facetLabel3->GetWidth(), facetLabel1->GetHeight());
	facetSettingsPanel->Add(facetMemoryEstimateLabel);
	



	applyButton = new GLButton(0,"Apply");
	applyButton->SetBounds(wD/2-50,hD-44,100,21);
	Add(applyButton);
	

	// Right center
	SetBounds(20,40,wD,hD); //Default position

	RestoreDeviceObjects();
	
	this->geom = geom;
	this->work = w;
}

bool HistogramSettings::Apply() {
	//Check input, return false if error, otherwise apply and return true
	size_t globalHitLimit, facetHitLimit, globalHitBinsize, facetHitBinsize;
	double globalDistanceLimit, facetDistanceLimit, globalDistanceBinsize, facetDistanceBinsize,
		globalTimeLimit, facetTimeLimit, globalTimeBinsize, facetTimeBinsize;

	bool globalRecBounce = globalRecordBounceToggle->GetState();
	work->wp.globalHistogramParams.recordBounce = globalRecBounce;

	if (globalRecBounce) {
		if (!globalHitLimitText->GetNumberSizeT(&globalHitLimit) || globalHitLimit < 0) {
			GLMessageBox::Display("Global bounce limit must be a non-negative integer", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		work->wp.globalHistogramParams.nbBounceMax = globalHitLimit;

		if (!globalHitBinsizeText->GetNumberSizeT(&globalHitBinsize) || globalHitBinsize < 1) {
			GLMessageBox::Display("Global bounce bin size must be a positive integer", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		work->wp.globalHistogramParams.nbBounceBinsize = globalHitBinsize;
	}

	bool globalRecDistance = globalRecordDistanceToggle->GetState();
	work->wp.globalHistogramParams.recordDistance = globalRecDistance;

	if (globalRecDistance) {
		if (!globalDistanceLimitText->GetNumber(&globalDistanceLimit) || globalDistanceLimit < 0) {
			GLMessageBox::Display("Global distance limit must be a non-negative scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		work->wp.globalHistogramParams.distanceMax = globalDistanceLimit;

		if (!globalDistanceBinsizeText->GetNumber(&globalDistanceBinsize) || globalDistanceBinsize <= 0) {
			GLMessageBox::Display("Global distance bin size must be a positive scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		work->wp.globalHistogramParams.distanceBinsize = globalDistanceBinsize;
	}

#ifdef MOLFLOW
	bool globalRecTime = globalRecordDistanceToggle->GetState();
	work->wp.globalHistogramParams.recordTime = globalRecTime;

	if (globalRecTime) {

		if (!globalTimeLimitText->GetNumber(&globalTimeLimit) || globalTimeLimit < 0) {
			GLMessageBox::Display("Global time limit must be a non-negative scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		work->wp.globalHistogramParams.timeMax = globalTimeLimit;

		if (!globalTimeBinsizeText->GetNumber(&globalTimeBinsize) || globalTimeBinsize <= 0) {
			GLMessageBox::Display("Global time bin size must be a positive scalar", "Histogram parameter error", GLDLG_OK, GLDLG_ICONERROR);
			return false;
		}
		work->wp.globalHistogramParams.timeBinsize = globalTimeBinsize;
	}
#endif

	//TODO: Apply facet parameters

	work->Reload();

	return true;
}

void HistogramSettings::Refresh(const std::vector<size_t>& selectedFacetIds) {
	//Update displayed info based on selected facets
	globalRecordBounceToggle->SetState(work->wp.globalHistogramParams.recordBounce);
	globalHitLimitText->SetText(work->wp.globalHistogramParams.nbBounceMax);
	globalHitBinsizeText->SetText(work->wp.globalHistogramParams.nbBounceBinsize);
	globalRecordDistanceToggle->SetState(work->wp.globalHistogramParams.recordDistance);
	globalDistanceLimitText->SetText(work->wp.globalHistogramParams.distanceMax);
	globalDistanceBinsizeText->SetText(work->wp.globalHistogramParams.distanceBinsize);
#ifdef MOLFLOW
	globalRecordTimeToggle->SetState(work->wp.globalHistogramParams.recordTime);
	globalTimeLimitText->SetText(work->wp.globalHistogramParams.timeMax);
	globalTimeBinsizeText->SetText(work->wp.globalHistogramParams.timeBinsize);
#endif

	bool hasFacetSelected = selectedFacetIds.size() > 0;
	if (!hasFacetSelected) {
		facetRecordBounceToggle->SetState(false);
		facetRecordDistanceToggle->SetState(false);
#ifdef MOLFLOW
		facetRecordTimeToggle->SetState(false);
#endif
	}
	facetRecordBounceToggle->SetEnabled(hasFacetSelected);
	facetRecordDistanceToggle->SetEnabled(hasFacetSelected);
	facetRecordTimeToggle->SetEnabled(hasFacetSelected);
	
	EnableDisableControls();
}

void HistogramSettings::EnableDisableControls() {
	globalHitLimitText->SetEditable(globalRecordBounceToggle->GetState());
	globalHitBinsizeText->SetEditable(globalRecordBounceToggle->GetState());
	globalDistanceLimitText->SetEditable(globalRecordDistanceToggle->GetState());
	globalDistanceBinsizeText->SetEditable(globalRecordDistanceToggle->GetState());
#ifdef MOLFLOW
	globalTimeLimitText->SetEditable(globalRecordTimeToggle->GetState());
	globalTimeBinsizeText->SetEditable(globalRecordTimeToggle->GetState());
#endif

	facetHitLimitText->SetEditable(facetRecordBounceToggle->GetState());
	facetHitBinsizeText->SetEditable(facetRecordBounceToggle->GetState());
	facetDistanceLimitText->SetEditable(facetRecordDistanceToggle->GetState());
	facetDistanceBinsizeText->SetEditable(facetRecordDistanceToggle->GetState());
#ifdef MOLFLOW
	facetTimeLimitText->SetEditable(facetRecordTimeToggle->GetState());
	facetTimeBinsizeText->SetEditable(facetRecordTimeToggle->GetState());
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
			break;
		
	}
	GLWindow::ProcessMessage(src,message);
}

