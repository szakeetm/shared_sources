#include "SmartSelection.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"

#include "Geometry_shared.h"

#include <sstream>

//#include "Worker.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

extern GLApplication *theApp;

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

SmartSelection::SmartSelection(InterfaceGeometry *g,Worker *w):GLWindow() {

	int wD = 270;
	int hD = 125;

	SetTitle("Smart selection");

	analyzeButton = new GLButton(0,"Analyze");
	analyzeButton->SetBounds(90,5,100,20);
	Add(analyzeButton);
	
	GLLabel *l1 = new GLLabel("Max. plane diff. between neighbors (deg):");
	l1->SetBounds(5, 30, 200, 20);
	Add(l1);

	angleThreshold = new GLTextField(0,"30");
	angleThreshold->SetBounds(210,30,40,18);
	Add(angleThreshold);

	resultLabel = new GLLabel("No neighborhood analysis yet.");
	resultLabel->SetBounds(50,55,150,20);
	Add(resultLabel);

	enableToggle = new GLToggle(0,"Enable smart selection");
	enableToggle->SetBounds(55,80,170,18);
	enableToggle->SetState(false);
	enableToggle->SetEnabled(false);
	Add(enableToggle);

	SetBounds(10,60,wD,hD);

	RestoreDeviceObjects();

	interfGeom = g;
	work = w;
	isRunning = false;
}

bool SmartSelection::IsSmartSelection()
{
	if (!IsVisible()) return false;
	else return (enableToggle->GetState()==1);
}

double SmartSelection::GetMaxAngle()
{
	if (!IsVisible()) return -1.0;
	double maxAngleDiff;
	if (!angleThreshold->GetNumber(&maxAngleDiff) || !(maxAngleDiff>=0.0)) {
		GLMessageBox::Display("Invalid angle threshold in Smart Selection dialog\nMust be a non-negative number.", "Error", GLDLG_OK, GLDLG_ICONERROR);
		return -1.0;
	}
	return maxAngleDiff/180.0*3.14159;
}

void SmartSelection::ProcessMessage(GLComponent *src,int message) {
	

	switch(message) {
	case MSG_BUTTON:

		if (src==analyzeButton) {
			if (!isRunning) {
				if (!interfGeom->IsLoaded()) {
					GLMessageBox::Display("Geometry not loaded yet", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				analyzeButton->SetText("Stop analyzing");
				isRunning = true;
				GLProgress_GUI prg("Analyzing facets", "Please wait");
				prg.SetClosable(false); //Use stop button
				prg.SetVisible(true);

				size_t nbAnalyzed = interfGeom->AnalyzeNeigbors(work,prg);
				analyzeButton->SetText("Analyze");
				isRunning = false;
				std::stringstream tmp;
				tmp << "Analyzed " << nbAnalyzed << " facets.";
				resultLabel->SetText(tmp.str().c_str());
				enableToggle->SetEnabled(true);
				enableToggle->SetState(1);
			}
			else {
				isRunning = false;
				work->abortRequested = true;
				analyzeButton->SetText("Analyze");
			}
		}
		break;
	case MSG_CLOSE:
		isRunning = false;
		work->abortRequested = true;
		analyzeButton->SetText("Analyze");
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

