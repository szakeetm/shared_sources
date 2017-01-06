/*
File:        ScaleVertex.cpp
Description: Mirror facet to plane dialog
Program:     MolFlow


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#define XYZMODE 0
#define VERTEXMODE 1
#define FACETMODE 2

#include "ScaleVertex.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
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

ScaleVertex::ScaleVertex(Geometry *g,Worker *w):GLWindow() {

	int wD = 275;
	int hD = 175;
	invariantMode=XYZMODE;

	SetTitle("Scale selected vertices");

	iPanel = new GLTitledPanel("Invariant point definiton mode");
	iPanel->SetBounds(5,3,wD-10,97);
	Add(iPanel);

	l1 = new GLToggle(0,"");
	l1->SetBounds(10,20,20,18);
	l1->SetState(TRUE);
	iPanel->Add(l1);

	GLLabel *xLabel = new GLLabel("X=");
	xLabel->SetBounds(30,20,20,18);
	iPanel->Add(xLabel);

	xText = new GLTextField(0,"0");
	xText->SetBounds(45,20,50,18);
	//xText->SetEditable(FALSE);
	iPanel->Add(xText);

	GLLabel *yLabel = new GLLabel("Y=");
	yLabel->SetBounds(100,20,20,18);
	iPanel->Add(yLabel);

	yText = new GLTextField(0,"0");
	yText->SetBounds(115,20,50,18);
	//yText->SetEditable(FALSE);
	iPanel->Add(yText);

	GLLabel* zLabel = new GLLabel("Z=");
	zLabel->SetBounds(170,20,20,18);
	iPanel->Add(zLabel);

	zText = new GLTextField(0,"0");
	zText->SetBounds(185,20,50,18);
	//zText->SetEditable(FALSE);
	iPanel->Add(zText);



	l2 = new GLToggle(0,"Vertex #");
	l2->SetBounds(10,45,100,18);
	iPanel->Add(l2);

	vertexNumber = new GLTextField(0,"0");
	vertexNumber->SetBounds(115,45,50,18);
	vertexNumber->SetEditable(FALSE);
	iPanel->Add(vertexNumber);

	l3 = new GLToggle(0,"Center of Facet #");
	l3->SetBounds(10,70,100,18);
	iPanel->Add(l3);

	facetNumber = new GLTextField(0,"0");
	facetNumber->SetBounds(115,70,50,18);
	facetNumber->SetEditable(FALSE);
	iPanel->Add(facetNumber);

	GLLabel *label4 = new GLLabel("Scale factor:");
	label4->SetBounds(10,105,100,18);
	Add(label4);

	factorNumber = new GLTextField(0,"1");
	factorNumber->SetBounds(115,105,50,18);
	//factorNumber->SetEditable(FALSE);
	Add(factorNumber);

	scaleButton = new GLButton(0,"Scale");
	scaleButton->SetBounds(5,hD-44,85,21);
	Add(scaleButton);

	copyButton = new GLButton(0,"Copy");
	copyButton->SetBounds(95,hD-44,85,21);
	Add(copyButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(185,hD-44,85,21);
	Add(cancelButton);

	// Center dialog
	int wS,hS;
	GLToolkit::GetScreenSize(&wS,&hS);
	int xD = (wS-wD)/2;
	int yD = (hS-hD)/2;
	SetBounds(xD,yD,wD,hD);

	RestoreDeviceObjects();

	geom = g;
	work = w;

}

void ScaleVertex::ProcessMessage(GLComponent *src,int message) {
	double x,y,z,factor;
	int facetNum,vertexNum;

	switch(message) {
		// -------------------------------------------------------------
	case MSG_TOGGLE:
		UpdateToggle(src);
		break;

	case MSG_BUTTON:

		if(src==cancelButton) {

			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==scaleButton || src==copyButton) {
			if (geom->GetNbSelectedVertex()==0) {
				GLMessageBox::Display("No vertices selected","Nothing to scale",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			Vector3d invariant;

			switch (invariantMode) {
			case XYZMODE:
				if( !(xText->GetNumber(&x))) {
					GLMessageBox::Display("Invalid X coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(yText->GetNumber(&y))) {
					GLMessageBox::Display("Invalid Y coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(zText->GetNumber(&z))) {
					GLMessageBox::Display("Invalid Z coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				invariant.x=x;
				invariant.y=y;
				invariant.z=z;
				break;
			case FACETMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				invariant=geom->GetFacet(facetNum-1)->sh.center;
				break;
			case VERTEXMODE:
				if( !(vertexNumber->GetNumberInt(&vertexNum))||vertexNum<1||vertexNum>geom->GetNbVertex() ) {
					GLMessageBox::Display("Invalid vertex number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				invariant=*(geom->GetVertex(vertexNum-1));
				break;
			default:
				GLMessageBox::Display("Select a plane definition mode.","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if( !(factorNumber->GetNumber(&factor))) {
				GLMessageBox::Display("Invalid scale factor number","Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if (mApp->AskToReset()) {
				geom->ScaleSelectedVertices(invariant,factor,src==copyButton,work);
				mApp->UpdateModelParams();
				try { work->Reload(); } catch(Error &e) {
					GLMessageBox::Display((char *)e.GetMsg(),"Error reloading worker",GLDLG_OK,GLDLG_ICONERROR);
				}  
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
				mApp->changedSinceSave = TRUE;
				//GLWindowManager::FullRepaint();
			}
		}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

void ScaleVertex::UpdateToggle(GLComponent *src) {
	l1->SetState(FALSE);
	l2->SetState(FALSE);
	l3->SetState(FALSE);

	GLToggle *toggle=(GLToggle*)src;
	toggle->SetState(TRUE);

	facetNumber->SetEditable(src==l3);
	vertexNumber->SetEditable(src==l2);
	xText->SetEditable(src==l1);
	yText->SetEditable(src==l1);
	zText->SetEditable(src==l1);

	if (src==l1) invariantMode=XYZMODE;
	if (src==l2) invariantMode=VERTEXMODE;
	if (src==l3) invariantMode=FACETMODE;
}