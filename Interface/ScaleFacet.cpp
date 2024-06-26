#define XYZMODE 0
#define VERTEXMODE 1
#define FACETMODE 2

#define UNIFORMMODE 0
#define DISTORTMODE 1

#include "ScaleFacet.h"

#include "Facet_shared.h"

#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"

#include "Geometry_shared.h"
#include "Worker.h"

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

ScaleFacet::ScaleFacet(InterfaceGeometry *g, Worker *w) :GLWindow() {

	int wD = 290;
	int hD = 220;
	invariantMode = XYZMODE;
	scaleMode = UNIFORMMODE;

	SetTitle("Scale selected facets");

	iPanel = new GLTitledPanel("Invariant point definiton mode");
	iPanel->SetBounds(5, 3, wD - 10, 97);
	Add(iPanel);

	l1 = new GLToggle(0, "");
	l1->SetBounds(10, 20, 20, 18);
	l1->SetState(true);
	iPanel->Add(l1);

	GLLabel *xLabel = new GLLabel("X=");
	xLabel->SetBounds(30, 20, 20, 18);
	iPanel->Add(xLabel);

	xText = new GLTextField(0, "0");
	xText->SetBounds(45, 20, 50, 18);
	//xText->SetEditable(false);
	iPanel->Add(xText);

	GLLabel *yLabel = new GLLabel("Y=");
	yLabel->SetBounds(100, 20, 20, 18);
	iPanel->Add(yLabel);

	yText = new GLTextField(0, "0");
	yText->SetBounds(115, 20, 50, 18);
	//yText->SetEditable(false);
	iPanel->Add(yText);

	GLLabel* zLabel = new GLLabel("Z=");
	zLabel->SetBounds(170, 20, 20, 18);
	iPanel->Add(zLabel);

	zText = new GLTextField(0, "0");
	zText->SetBounds(185, 20, 50, 18);
	//zText->SetEditable(false);
	iPanel->Add(zText);

	l2 = new GLToggle(0, "Selected Vertex");
	l2->SetBounds(10, 45, 100, 18);
	iPanel->Add(l2);

	l3 = new GLToggle(0, "Center of Facet #");
	l3->SetBounds(10, 70, 100, 18);
	iPanel->Add(l3);

	facetNumber = new GLTextField(0, "0");
	facetNumber->SetBounds(115, 70, 60, 18);
	facetNumber->SetEditable(false);
	iPanel->Add(facetNumber);

	getSelFacetButton = new GLButton(0, "<-Get selected");
	getSelFacetButton->SetBounds(180, 70, 80, 18);
	iPanel->Add(getSelFacetButton);

	sPanel = new GLTitledPanel("Scale factor");
	sPanel->SetBounds(5, 101, wD - 10, 65);
	Add(sPanel);

	uniform = new GLToggle(0, "Uniform");
	uniform->SetBounds(10, 115, 100, 18);
	uniform->SetState(true);
	Add(uniform);

	factorNumber = new GLTextField(0, "1");
	factorNumber->SetBounds(115, 115, 60, 18);
	//factorNumber->SetEditable(false);
	Add(factorNumber);

	GLLabel *onePerLabel = new GLLabel("(= 1/                )");
	onePerLabel->SetBounds(190, 115, 90, 18);
	Add(onePerLabel);

	OnePerFactor = new GLTextField(0, "");
	OnePerFactor->SetBounds(215, 115, 60, 18);
	Add(OnePerFactor);

	distort = new GLToggle(0, "Distorted");
	distort->SetBounds(10, 140, 100, 18);
	Add(distort);

	GLLabel* x2Label = new GLLabel("X:");
	x2Label->SetBounds(103, 141, 20, 18);
	Add(x2Label);

	factorNumberX = new GLTextField(0, "1");
	factorNumberX->SetBounds(115, 140, 40, 18);
	factorNumberX->SetEditable(false);
	Add(factorNumberX);

	GLLabel* y2Label = new GLLabel("Y:");
	y2Label->SetBounds(163, 141, 20, 18);
	Add(y2Label);

	factorNumberY = new GLTextField(0, "1");
	factorNumberY->SetBounds(175, 140, 40, 18);
	factorNumberY->SetEditable(false);
	Add(factorNumberY);

	GLLabel* z2Label = new GLLabel("Z:");
	z2Label->SetBounds(223, 141, 20, 18);
	Add(z2Label);

	factorNumberZ = new GLTextField(0, "1");
	factorNumberZ->SetBounds(235, 140, 40, 18);
	factorNumberZ->SetEditable(false);
	Add(factorNumberZ);

	scaleButton = new GLButton(0, "Scale facet");
	scaleButton->SetBounds(5, hD - 44, 85, 21);
	Add(scaleButton);

	copyButton = new GLButton(0, "Copy facet");
	copyButton->SetBounds(95, hD - 44, 85, 21);
	Add(copyButton);

	cancelButton = new GLButton(0, "Dismiss");
	cancelButton->SetBounds(185, hD - 44, 85, 21);
	Add(cancelButton);

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);

	RestoreDeviceObjects();

	interfGeom = g;
	work = w;

}

void ScaleFacet::ProcessMessage(GLComponent *src, int message) {
	double x, y, z, factor, factorX, factorY, factorZ;
	int facetNum;

	switch (message) {
		
	case MSG_TOGGLE:
		UpdateToggle(src);
		break;

	case MSG_BUTTON:

		if (src == cancelButton) {

			GLWindow::ProcessMessage(NULL, MSG_CLOSE);

		}
		else if (src == scaleButton || src == copyButton) {
			if (interfGeom->GetNbSelectedFacets() == 0) {
				GLMessageBox::Display("No facets selected", "Nothing to scale", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}

			Vector3d invariant;
			bool found;

			switch (invariantMode) {
			case XYZMODE:
				if (!(xText->GetNumber(&x))) {
					GLMessageBox::Display("Invalid X coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(yText->GetNumber(&y))) {
					GLMessageBox::Display("Invalid Y coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				if (!(zText->GetNumber(&z))) {
					GLMessageBox::Display("Invalid Z coordinate", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				invariant.x = x;
				invariant.y = y;
				invariant.z = z;
				break;
			case FACETMODE:
				if (!(facetNumber->GetNumberInt(&facetNum)) || facetNum<1 || facetNum>interfGeom->GetNbFacet()) {
					GLMessageBox::Display("Invalid facet number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				invariant = interfGeom->GetFacet(facetNum - 1)->sh.center;
				break;
			case VERTEXMODE:
				if (!(interfGeom->GetNbSelectedVertex() == 1)) {
					GLMessageBox::Display("Select exactly one vertex", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
				found = false;
				for (int i = 0; !found && i < interfGeom->GetNbVertex(); i++) {
					if (interfGeom->GetVertex(i)->selected)
						invariant = *(interfGeom->GetVertex(i));
				}
				break;
			default:
				GLMessageBox::Display("Select an invariant definition mode.", "Error", GLDLG_OK, GLDLG_ICONERROR);
				return;
			}

			if (scaleMode == UNIFORMMODE) {
				if (!(factorNumber->GetNumber(&factor))) {
					GLMessageBox::Display("Invalid scale factor number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
			}
			else {
				if (!(factorNumberX->GetNumber(&factorX))) {
					GLMessageBox::Display("Invalid X scale factor number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}

				if (!(factorNumberY->GetNumber(&factorY))) {
					GLMessageBox::Display("Invalid Y scale factor number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}

				if (!(factorNumberZ->GetNumber(&factorZ))) {
					GLMessageBox::Display("Invalid Z scale factor number", "Error", GLDLG_OK, GLDLG_ICONERROR);
					return;
				}
			}
			if (mApp->AskToReset()) {
				if (scaleMode == UNIFORMMODE) factorX = factorY = factorZ = factor;
				interfGeom->ScaleSelectedFacets(invariant, factorX, factorY, factorZ, src == copyButton, work);
				mApp->UpdateModelParams();
				work->MarkToReload();
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
				mApp->changedSinceSave = true;
				//GLWindowManager::FullRepaint();
			}
		}
		else if (src == getSelFacetButton) {
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
			facetNumber->SetText(selFacetId + 1);
			UpdateToggle(l3);
		}
		break;
	case MSG_TEXT_UPD:
		if (src == factorNumber) {
			double factor;
			if (factorNumber->GetNumber(&factor)) {
				OnePerFactor->SetText(1.0 / factor);
			}
		}
		else if (src == OnePerFactor) {
			double onePerFactor;
			if (OnePerFactor->GetNumber(&onePerFactor)) {
				factorNumber->SetText(1.0 / onePerFactor);
			}
		}
		break;
	}

	GLWindow::ProcessMessage(src, message);
}

void ScaleFacet::UpdateToggle(GLComponent *src) {

	if (src == l1 || src == l2 || src == l3) {

		l1->SetState(src == l1);
		l2->SetState(src == l2);
		l3->SetState(src == l3);

		facetNumber->SetEditable(src == l3);
		xText->SetEditable(src == l1);
		yText->SetEditable(src == l1);
		zText->SetEditable(src == l1);

		if (src == l1) invariantMode = XYZMODE;
		if (src == l2) invariantMode = VERTEXMODE;
		if (src == l3) invariantMode = FACETMODE;
	}
	else {
		uniform->SetState(src == uniform);
		distort->SetState(src == distort);

		factorNumber->SetEditable(src == uniform);
		OnePerFactor->SetEditable(src == uniform);
		factorNumberX->SetEditable(src == distort);
		factorNumberY->SetEditable(src == distort);
		factorNumberZ->SetEditable(src == distort);

		if (src == uniform) scaleMode = UNIFORMMODE;
		if (src == distort) scaleMode = DISTORTMODE;
	}
}