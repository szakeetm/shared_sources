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

	recordToggle = new GLToggle(0,"Record histogram on selected facet(s)");
	recordToggle->SetBounds(5,5,170,18);
	recordToggle->AllowMixedState(true);
	recordToggle->SetState(true);
	Add(recordToggle);

	GLLabel* l1 = new GLLabel("Max recorded flight distance (cm):");
	l1->SetBounds(10, 30, 150, 20);
	Add(l1);

	distanceLimit = new GLTextField(0,"1000");
	distanceLimit->SetBounds(185,30,80,18);
	Add(distanceLimit);

	GLLabel* l2 = new GLLabel("Max recorded no. of hits:");
	l2->SetBounds(10, 55, 150, 20);
	Add(l2);

	hitLimit = new GLTextField(0,"1000");
	hitLimit->SetBounds(185,55,80,18);
	Add(hitLimit);

	GLLabel* l3 = new GLLabel("Hit bin size:");
	l3->SetBounds(10, 80, 150, 20);
	Add(l3);

	hitBinSize = new GLTextField(0,"1");
	hitBinSize->SetBounds(185,80,80,18);
	Add(hitBinSize);

	memoryEstimateLabel = new GLLabel("Memory estimate of selected facets:");
	memoryEstimateLabel->SetBounds(10,105,wD-20,20);
	Add(memoryEstimateLabel);

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

void HistogramSettings::Reposition() {
	int refX, refY, w, h;
	mApp->facetHistogramBtn->GetBounds(&refX, &refY, &w, &h);
	SetBounds(refX - GetWidth() - 115, refY + 5, GetWidth(), GetHeight());
}

bool HistogramSettings::Apply() {
	//Check input, return false if error, otherwise apply and return true
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

