/*
File:        AlignFacet.cpp
Description: Align facet to an other dialog
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

#include "AlignFacet.h"
#include "Facet.h"
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

AlignFacet::~AlignFacet() {
	SAFE_FREE(selection);
	for( int i = 0 ; i < nbSelected ; i++ )
		SAFE_FREE(oriPos[i]) ;
	SAFE_FREE(oriPos);
}

AlignFacet::AlignFacet(Geometry *g,Worker *w):GLWindow() {

	int wD = 290;
	int hD = 355;
	oriPos=NULL;

	nbMemo=0;

	SetTitle("Align selected facets to an other");

	step1 = new GLTitledPanel("Step 1: select facets of the object");
	step1->SetBounds(5,5,wD-10,75);
	Add(step1);
	
	numFacetSel = new GLLabel("0 facets will be aligned");
	numFacetSel->SetBounds(75,25,120,21);
	step1->Add(numFacetSel);
	
	memoSel = new GLButton(0,"Update from selection");
	memoSel->SetBounds(70,50,130,21);
	step1->Add(memoSel);

	step2 = new GLTitledPanel("Step 2: select snapping facets & points");
	step2->SetBounds(5,85,wD-10,115);
	Add(step2);

	l1 = new GLLabel("1. Choose two facets that will be snapped together.\n\n2. Choose 2-2 vertices on the source and destination \nfacets: One will serve as an anchor point, one as a\ndirection aligner. Once you have 2 facets and 4\nvertices selected, proceed to step 3.");
	l1->SetBounds(10,100,120,21);
	step2->Add(l1);

	step3 = new GLTitledPanel("Step 3: align");
	step3->SetBounds(5,205,wD-10,120);
	Add(step3);

	invertNormal = new GLToggle(0,"Invert normal");
	invertNormal->SetBounds(10,220,150,21);
	invertNormal->SetState(TRUE);
	step3->Add(invertNormal);

	invertDir1 = new GLToggle(0,"Swap anchor/direction vertices on source");
	invertDir1->SetBounds(10,245,150,21);
	step3->Add(invertDir1);

	invertDir2 = new GLToggle(0,"Swap ancor/direction vertices on destination");
	invertDir2->SetBounds(10,270,150,21);
	step3->Add(invertDir2);

	alignButton = new GLButton(0,"Align");
	alignButton->SetBounds(10,295,63,21);
	step3->Add(alignButton);

	copyButton = new GLButton(0,"Copy");
	copyButton->SetBounds(78,295,63,21);
	//step3->Add(copyButton);

	undoButton = new GLButton(0,"Undo");
	undoButton->SetBounds(146,295,63,21);
	step3->Add(undoButton);

	cancelButton = new GLButton(0,"Dismiss");
	cancelButton->SetBounds(214,295,63,21);
	step3->Add(cancelButton);

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

void AlignFacet::ProcessMessage(GLComponent *src,int message) {
	switch(message) {
	case MSG_BUTTON:

		if(src==cancelButton) {
			GLWindow::ProcessMessage(NULL,MSG_CLOSE);

		} else if (src==memoSel) {
			MemorizeSelection();
		} else if (src==alignButton || src==copyButton) {
			if (nbMemo==0) {
				GLMessageBox::Display("No facets memorized","Nothing to align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			int nbSelected=geom->GetNbSelected();
			if (nbSelected!=2) {
				GLMessageBox::Display("Two facets (source and destination) must be selected","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			int Facet_source=-1;
			for (int i=0;i<nbMemo;i++) { //find source facet
				if (geom->GetFacet(selection[i])->selected) {
					if (Facet_source==-1) {
						Facet_source=selection[i];
					} else {
						GLMessageBox::Display("Both selected facets are on the source object. One must be on the destination.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
						return;
					}
				}
			}
			if (Facet_source==-1) {
				GLMessageBox::Display("No facet selected on source object.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			int Facet_dest=-1;
			for (int i=0;i<geom->GetNbFacet()&&Facet_dest==-1;i++) { //find destination facet
				if (geom->GetFacet(i)->selected && i!=Facet_source) Facet_dest=i;
			}
			if (Facet_dest==-1) {
				GLMessageBox::Display("Can't find destination facet","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if (geom->GetNbSelectedVertex()!=4) {
				GLMessageBox::Display("4 vertices must be selected: two on source and two on destination facets","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			int Anchor_source,Anchor_dest,Dir_source,Dir_dest;
			Anchor_source=Anchor_dest=Dir_source=Dir_dest=-1;

			//find source anchor and dir vertex
			for (int j=0;j<geom->GetFacet(Facet_source)->sh.nbIndex;j++) {
				if (geom->GetVertex(geom->GetFacet(Facet_source)->indices[j])->selected) {
					if (Anchor_source==-1 && Dir_source==-1) {
						Anchor_source=geom->GetFacet(Facet_source)->indices[j];
					} else if (Dir_source==-1) {
						Dir_source=geom->GetFacet(Facet_source)->indices[j];
					} else {
						GLMessageBox::Display("More than two selected vertices are on the source facet. Two must be on the destination.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
						return;
					}
				}
			}

			if (Anchor_source==-1 || Dir_source==-1) {
				GLMessageBox::Display("Less than two selected vertices found on source facet. Select two (anchor, direction).","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			//find destination anchor and dir vertex
			for (int j=0;j<geom->GetFacet(Facet_dest)->sh.nbIndex;j++) {
				if (geom->GetVertex(geom->GetFacet(Facet_dest)->indices[j])->selected) {
					if (Anchor_dest==-1 && Dir_dest==-1) {
						Anchor_dest=geom->GetFacet(Facet_dest)->indices[j];
					} else if (Dir_dest==-1) {
						Dir_dest=geom->GetFacet(Facet_dest)->indices[j];
					} else {
						GLMessageBox::Display("More than two selected vertices are on the destination facet. Two must be on the source.","Can't align",GLDLG_OK,GLDLG_ICONERROR);
						return;
					}
				}

			}
			if (Anchor_dest==-1 || Dir_dest==-1) {
				GLMessageBox::Display("Less than two selected vertices found on destination facet. Select two (anchor, direction).","Can't align",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}

			if (mApp->AskToReset()){
				geom->AlignFacets(selection,nbMemo,Facet_source,Facet_dest,Anchor_source,Anchor_dest,Dir_source,Dir_dest,
					invertNormal->GetState(),invertDir1->GetState(),invertDir2->GetState(),src==copyButton,work);
				#ifdef MOLFLOW
				if (src == copyButton) mApp->worker.GetGeometry(); mApp->worker.CalcTotalOutgassing();
				#endif
				//mApp->UpdateModelParams();
				try { work->Reload(); } catch(Error &e) {

					GLMessageBox::Display((char *)e.GetMsg(),"Error reloading worker",GLDLG_OK,GLDLG_ICONERROR);
				}
				mApp->changedSinceSave = TRUE;
				mApp->UpdateFacetlistSelected();	
				mApp->UpdateViewers();
				//GLWindowManager::FullRepaint();
			}
		} else if (src==undoButton) {
			if (!mApp->AskToReset(work)) return;
			for (int i=0;i<nbSelected;i++) {
				Facet *f=geom->GetFacet(selection[i]);
				for (int j=0;j<f->sh.nbIndex;j++) {
					geom->GetVertex(f->indices[j])->SetLocation(this->oriPos[i][j]);
				}
			}
			geom->InitializeGeometry();
			//for(int i=0;i<nbSelected;i++)

			//	geom->SetFacetTexture(selection[i],geom->GetFacet(selection[i])->tRatio,geom->GetFacet(selection[i])->hasMesh);	
			work->Reload();			 
			mApp->UpdateFacetlistSelected();	
			mApp->UpdateViewers();
		}
		break;
	}

	GLWindow::ProcessMessage(src,message);
}

void AlignFacet::MemorizeSelection() {
	nbSelected=geom->GetNbSelected();
	selection = (int*)malloc(nbSelected*sizeof(int));
	//oriPos=new Vector3d *[nbSelected];
	oriPos=(Vector3d**)malloc(nbSelected*sizeof(Vector3d*));
	int sel=0;
	for (int i=0;i<geom->GetNbFacet();i++) {
		Facet *f=geom->GetFacet(i);
		if (f->selected) {
			selection[sel++]=i;
			oriPos[sel-1]=(Vector3d*)malloc(f->sh.nbIndex*sizeof(Vector3d));
			for (int j=0;j<f->sh.nbIndex;j++) 
				oriPos[sel-1][j]=*(geom->GetVertex(f->indices[j]));
		}
	}
	char tmp[256];
	sprintf(tmp,"%d facets will be aligned.",nbSelected);
	numFacetSel->SetText(tmp);
	nbMemo=nbSelected;
}