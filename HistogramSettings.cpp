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
	int panelHeight = 165;
	int hD = 2*panelHeight+85;

#ifdef MOLFLOW
	hD += 4 * 25; //Time parameters
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

	globalRecordToggle = new GLToggle(0, "Record global histogram");
	globalSettingsPanel->SetCompBounds(globalRecordToggle, 5, 15, 175, 25);
	globalSettingsPanel->Add(globalRecordToggle);

	GLLabel* globalLabel1 = new GLLabel("Max recorded no. of bounces:");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalRecordToggle, globalLabel1, 0, 25, globalRecordToggle->GetWidth(), 20);
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
	
	GLLabel* globalLabel3 = new GLLabel("Max recorded flight distance (cm):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel2, globalLabel3, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
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
	GLLabel* globalLabel5 = new GLLabel("Max recorded flight time (s):");
	globalSettingsPanel->SetCompBoundsRelativeTo(globalLabel4, globalLabel5, 0, 25, globalLabel1->GetWidth(), globalLabel1->GetHeight());
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


	//facet histogram settings
	facetSettingsPanel = new GLTitledPanel("Selected facets' histograms");
	SetCompBoundsRelativeTo(globalSettingsPanel, facetSettingsPanel, 0, globalSettingsPanel->GetHeight() + 5, globalSettingsPanel->GetWidth(), globalSettingsPanel->GetHeight());
	Add(facetSettingsPanel);

	facetRecordToggle = new GLToggle(0, "Record histogram on selected facets");
	facetSettingsPanel->SetCompBounds(facetRecordToggle, 5, 15, 175, 25);
	facetSettingsPanel->Add(facetRecordToggle);

	GLLabel* facetLabel1 = new GLLabel("Max recorded no. of bounces:");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetRecordToggle, facetLabel1, 0, 25, facetRecordToggle->GetWidth(), 20);
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

	GLLabel* facetLabel3 = new GLLabel("Max recorded flight distance (cm):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel2, facetLabel3, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
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
	GLLabel* facetLabel5 = new GLLabel("Max recorded flight time (s):");
	facetSettingsPanel->SetCompBoundsRelativeTo(facetLabel4, facetLabel5, 0, 25, facetLabel1->GetWidth(), facetLabel1->GetHeight());
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

	facetMemoryEstimateLabel = new GLLabel("Memory estimate of facet histograms:");
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

	//TODO: Validate input!
	work->wp.globalHistogramParams.record = globalRecordToggle->GetState();
	
	globalHitLimitText->GetNumberSizeT(&globalHitLimit);
	work->wp.globalHistogramParams.nbBounceMax = globalHitLimit;

	globalHitBinsizeText->GetNumberSizeT(&globalHitBinsize);
	work->wp.globalHistogramParams.nbBounceBinsize = globalHitBinsize;

	globalDistanceLimitText->GetNumber(&globalDistanceLimit);
	work->wp.globalHistogramParams.distanceMax = globalDistanceLimit;

	globalDistanceBinsizeText->GetNumber(&globalDistanceBinsize);
	work->wp.globalHistogramParams.distanceBinsize = globalDistanceBinsize;

	globalTimeLimitText->GetNumber(&globalTimeLimit);
	work->wp.globalHistogramParams.timeMax = globalTimeLimit;

	globalTimeBinsizeText->GetNumber(&globalTimeBinsize);
	work->wp.globalHistogramParams.timeBinsize = globalTimeBinsize;
	
	work->Reload();

	return true;
}

void HistogramSettings::Refresh(const std::vector<size_t>& selectedFacetIds) {
	//Update displayed info based on selected facets
	globalRecordToggle->SetState(work->wp.globalHistogramParams.record);
	globalHitLimitText->SetText(work->wp.globalHistogramParams.nbBounceMax);
	globalHitBinsizeText->SetText(work->wp.globalHistogramParams.nbBounceBinsize);
	globalDistanceLimitText->SetText(work->wp.globalHistogramParams.distanceMax);
	globalDistanceBinsizeText->SetText(work->wp.globalHistogramParams.distanceBinsize);
	globalTimeLimitText->SetText(work->wp.globalHistogramParams.timeMax);
	globalTimeBinsizeText->SetText(work->wp.globalHistogramParams.timeBinsize);

	bool hasFacetSelected = selectedFacetIds.size() > 0;
	if (!hasFacetSelected) facetRecordToggle->SetState(false);
	facetRecordToggle->SetEnabled(hasFacetSelected);
	
	EnableDisableControls();
}

void HistogramSettings::EnableDisableControls() {
	globalHitLimitText->SetEditable(globalRecordToggle->GetState());
	globalHitBinsizeText->SetEditable(globalRecordToggle->GetState());
	globalDistanceLimitText->SetEditable(globalRecordToggle->GetState());
	globalDistanceBinsizeText->SetEditable(globalRecordToggle->GetState());
#ifdef MOLFLOW
	globalTimeLimitText->SetEditable(globalRecordToggle->GetState());
	globalTimeBinsizeText->SetEditable(globalRecordToggle->GetState());
#endif

	facetHitLimitText->SetEditable(facetRecordToggle->GetState());
	facetHitBinsizeText->SetEditable(facetRecordToggle->GetState());
	facetDistanceLimitText->SetEditable(facetRecordToggle->GetState());
	facetDistanceBinsizeText->SetEditable(facetRecordToggle->GetState());
#ifdef MOLFLOW
	facetTimeLimitText->SetEditable(facetRecordToggle->GetState());
	facetTimeBinsizeText->SetEditable(facetRecordToggle->GetState());
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

