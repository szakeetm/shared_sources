/*
  File:        MirrorFacet.cpp
  Description: Mirror facet to plane dialog
  Program:     SynRad


  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#define XYMODE 0
#define XZMODE 1
#define YZMODE 2
#define FACETNMODE 3
#define THREEVERTEXMODE 4
#define ABCDMODE 5

#include "MirrorFacet.h"
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

extern GLApplication *theApp;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif
int    planeMode;

MirrorFacet::MirrorFacet(Geometry *g,Worker *w):GLWindow() {
	
  int wD = 275;
  int hD = 250;

  SetTitle("Mirror selected facets to a plane");

  iPanel = new GLTitledPanel("Plane definiton mode");
  iPanel->SetBounds(5,5,wD-10,190);
  Add(iPanel);

  l1 = new GLToggle(0,"XY plane");
  l1->SetBounds(10,20,100,18);
  iPanel->Add(l1);

  l2 = new GLToggle(0,"YZ plane");
  l2->SetBounds(10,45,100,18);
  iPanel->Add(l2);
  
  l3 = new GLToggle(0,"XZ plane");
  l3->SetBounds(10,70,100,18);
  iPanel->Add(l3);

  l4 = new GLToggle(0,"Mirror to facet #");
  l4->SetBounds(10,95,100,18);
  iPanel->Add(l4);

  facetNumber = new GLTextField(0,"0");
  facetNumber->SetBounds(105,95,60,18);
  facetNumber->SetEditable(FALSE);
  iPanel->Add(facetNumber);

  l5 = new GLToggle(0,"Define by 3 selected vertex");
  l5->SetBounds(10,120,100,18);
  iPanel->Add(l5);

  l6 = new GLToggle(0,"Define by plane equation:");
  l6->SetBounds(10,145,100,18);
  iPanel->Add(l6);

  aText = new GLTextField(0,"0");
  aText->SetBounds(15,170,30,18);
  aText->SetEditable(FALSE);
  iPanel->Add(aText);

  aLabel = new GLLabel("*X +");
  aLabel->SetBounds(50,170,20,18);
  iPanel->Add(aLabel);

    bText = new GLTextField(0,"0");
  bText->SetBounds(75,170,30,18);
  bText->SetEditable(FALSE);
  iPanel->Add(bText);

    bLabel = new GLLabel("*Y +");
  bLabel->SetBounds(110,170,20,18);
  iPanel->Add(bLabel);

    cText = new GLTextField(0,"0");
  cText->SetBounds(135,170,30,18);
  cText->SetEditable(FALSE);
  iPanel->Add(cText);

    cLabel = new GLLabel("*Z +");
  cLabel->SetBounds(170,170,20,18);
  iPanel->Add(cLabel);

    dText = new GLTextField(0,"0");
  dText->SetBounds(195,170,30,18);
  dText->SetEditable(FALSE);
  iPanel->Add(dText);

    dLabel = new GLLabel("= 0");
  dLabel->SetBounds(230,170,20,18);
  iPanel->Add(dLabel);

  moveButton = new GLButton(0,"Mirror");
  moveButton->SetBounds(5,hD-44,85,21);
  Add(moveButton);

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

void MirrorFacet::ProcessMessage(GLComponent *src,int message) {
  double a,b,c,d;
  int facetNum;

  switch(message) {
	// -------------------------------------------------------------
    case MSG_TOGGLE:
      UpdateToggle(src);
      break;

	case MSG_BUTTON:

    if(src==cancelButton) {

      GLWindow::ProcessMessage(NULL,MSG_CLOSE);

    } else if (src==moveButton || src==copyButton) {
		if (geom->GetNbSelected()==0) {
			GLMessageBox::Display("No facets selected","Nothing to mirror",GLDLG_OK,GLDLG_ICONERROR);
			return;
		}
			//Calculate the plane
			VERTEX3D P0,N;
			double nU2,nV2,nN2;
			int nbSelectedVertex;
			int *vIdx = (int *)malloc(geom->GetNbVertex()*sizeof(int));
			memset(vIdx,0xFF,geom->GetNbVertex()*sizeof(int));
			switch (planeMode) {
			case XYMODE:
				P0.x=0.0;P0.y=0.0;P0.z=0.0;
				N.x=0.0;N.y=0.0;N.z=1.0;
				break;
			case XZMODE:
				P0.x=0.0;P0.y=0.0;P0.z=0.0;
				N.x=0.0;N.y=1.0;N.z=0.0;
				break;
			case YZMODE:
				P0.x=0.0;P0.y=0.0;P0.z=0.0;
				N.x=1.0;N.y=0.0;N.z=0.0;
				break;
			case FACETNMODE:
				if( !(facetNumber->GetNumberInt(&facetNum))||facetNum<1||facetNum>geom->GetNbFacet() ) {
					GLMessageBox::Display("Invalid facet number","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				P0=*geom->GetVertex(geom->GetFacet(facetNum-1)->indices[0]);
				N=geom->GetFacet(facetNum-1)->sh.N;
				break;
			case THREEVERTEXMODE:
				if (geom->GetNbSelectedVertex()!=3) {
						        GLMessageBox::Display("Select exactly 3 vertices","Can't define plane",GLDLG_OK,GLDLG_ICONERROR);
								return;
				}
				nbSelectedVertex = 0;

				for(int i=0;i<geom->GetNbVertex()&&nbSelectedVertex<geom->GetNbSelectedVertex();i++ ) {
				//VERTEX3D *v = GetVertex(i);
					if( geom->GetVertex(i)->selected ) {
						vIdx[nbSelectedVertex] = i;
						nbSelectedVertex++;
				 }
				}

				VERTEX3D U2,V2,N2;
				U2.x = geom->GetVertex(vIdx[0])->x - geom->GetVertex(vIdx[1])->x;
				U2.y = geom->GetVertex(vIdx[0])->y - geom->GetVertex(vIdx[1])->y;
				U2.z = geom->GetVertex(vIdx[0])->z - geom->GetVertex(vIdx[1])->z;
				nU2 = Norme(U2);
				ScalarMult(&U2,1.0/nU2); // Normalize U2

				V2.x = geom->GetVertex(vIdx[0])->x - geom->GetVertex(vIdx[2])->x;
				V2.y = geom->GetVertex(vIdx[0])->y - geom->GetVertex(vIdx[2])->y;
				V2.z = geom->GetVertex(vIdx[0])->z - geom->GetVertex(vIdx[2])->z;
				nV2 = Norme(V2);
				ScalarMult(&V2,1.0/nV2); // Normalize V2

				Cross(&N2,&V2,&U2); //We have a normal vector
				nN2 = Norme(N2);
				if (nN2<1e-8) {
					GLMessageBox::Display("The 3 selected vertices are on a line.","Can't define plane",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				ScalarMult(&N2,1.0/nN2); // Normalize N2
				N=N2;
				P0=*(geom->GetVertex(vIdx[0]));
				break;
			case ABCDMODE:
				if( !(aText->GetNumber(&a)) ) {
					GLMessageBox::Display("Invalid A coefficient","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(bText->GetNumber(&b)) ) {
					GLMessageBox::Display("Invalid B coefficient","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(cText->GetNumber(&c)) ) {
					GLMessageBox::Display("Invalid C coefficient","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if( !(dText->GetNumber(&d)) ) {
					GLMessageBox::Display("Invalid D coefficient","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				if ((a==0.0)&&(b==0.0)&&(c==0.0)&&(d==0.0)) {
					GLMessageBox::Display("A, B, C are all zero. That's not a plane.","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
				}
				N.x=a;N.y=b;N.z=c;
				P0.x=0.0;P0.y=0;P0.z=0;
				if (!a==0) P0.x=-d/a;
				else if (!b==0) P0.y=-d/b;
				else if (!c==0) P0.z=-d/c;
			break;
			default:
					GLMessageBox::Display("Select a plane definition mode.","Error",GLDLG_OK,GLDLG_ICONERROR);
					return;
			}
			SAFE_FREE(vIdx);
			if (mApp->AskToReset()) {
				geom->MirrorSelectedFacets(P0,N,src==copyButton,work);
				//theApp->UpdateModelParams();
				try { work->Reload(); } catch(Error &e) {
					GLMessageBox::Display((char *)e.GetMsg(),"Error reloading worker",GLDLG_OK,GLDLG_ICONERROR);
				}  
				mApp->UpdateFacetlistSelected();
				mApp->UpdateViewers();
	       		//GLWindowManager::FullRepaint();
			}
    }
    break;
  }

  GLWindow::ProcessMessage(src,message);
}

void MirrorFacet::UpdateToggle(GLComponent *src) {
	l1->SetState(FALSE);
	l2->SetState(FALSE);
	l3->SetState(FALSE);
	l4->SetState(FALSE);
	l5->SetState(FALSE);
	l6->SetState(FALSE);

	GLToggle *toggle=(GLToggle*)src;
	toggle->SetState(TRUE);

	facetNumber->SetEditable(src==l4);
	aText->SetEditable(src==l6);
	bText->SetEditable(src==l6);
	cText->SetEditable(src==l6);
	dText->SetEditable(src==l6);

	if (src==l1) planeMode=XYMODE;
	if (src==l2) planeMode=YZMODE;
	if (src==l3) planeMode=XZMODE;
	if (src==l4) planeMode=FACETNMODE;
	if (src==l5) planeMode=THREEVERTEXMODE;
	if (src==l6) planeMode=ABCDMODE;

}