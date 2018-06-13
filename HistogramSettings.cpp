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

HistogramSettings::HistogramSettings():GLWindow() {

	int wD = 270;
	int hD = 150;

	SetTitle("Facet histogram settings");

	facetRecordToggle = new GLToggle(0,"Record histogram on selected facet(s)");
	facetRecordToggle->SetBounds(5,5,170,18);
	facetRecordToggle->AllowMixedState(true);
	facetRecordToggle->SetState(true);
	Add(facetRecordToggle);

	GLLabel* l1 = new GLLabel("Max recorded flight distance (cm):");
	l1->SetBounds(10, 30, 150, 20);
	Add(l1);

	facetDistanceLimitText = new GLTextField(0,"1000");
	facetDistanceLimitText->SetBounds(185,30,80,18);
	Add(facetDistanceLimitText);

	GLLabel* l2 = new GLLabel("Max recorded no. of hits:");
	l2->SetBounds(10, 55, 150, 20);
	Add(l2);

	facetHitLimitText = new GLTextField(0,"1000");
	facetHitLimitText->SetBounds(185,55,80,18);
	Add(facetHitLimitText);

	GLLabel* l3 = new GLLabel("Hit bin size:");
	l3->SetBounds(10, 80, 150, 20);
	Add(l3);

	facetHitBinSizeText = new GLTextField(0,"1");
	facetHitBinSizeText->SetBounds(185,80,80,18);
	Add(facetHitBinSizeText);

	facetMemoryEstimateLabel = new GLLabel("Memory estimate of selected facets:");
	facetMemoryEstimateLabel->SetBounds(10,105,wD-20,20);
	Add(facetMemoryEstimateLabel);

	/*
	applyButton = new GLButton(0,"Apply");
	applyButton->SetBounds(wD/2-50,hD-44,100,21);
	Add(applyButton);
	*/

	// Right center
	SetBounds(20,40,wD,hD); //Default position

	RestoreDeviceObjects();
	
	geom = NULL;
	work = NULL;
	
}

bool HistogramSettings::Apply() {
	//Check input, return false if error, otherwise apply and return true
	int globalHitLimit, facetHitLimit, globalHitBinSize, facetHitBinSize;
	double globalDistanceLimit, facetDistanceLimit;
	
	return true;
}

void HistogramSettings::SetGeometry(Geometry *geom,Worker *w) {

	this->geom = geom;
	work = w;

}

void HistogramSettings::Refresh(const std::vector<size_t>& selectedFacetIds) {
	//Update displayed info based on selected facets
	
}

void HistogramSettings::ProcessMessage(GLComponent *src,int message) {
	

	switch (message) {
		/*
		case MSG_BUTTON:

			if (src==applyButton) {
				//Set histogram parameters on selected facets

			}
			break;
		}
		*/
	}
	GLWindow::ProcessMessage(src,message);
}

