/*
  File:        FacetCoordinates.cpp
  Description: Facet coordinates window
  Program:     MolFlow
  Author:      R. Kerservan / J-L PONS / M ADY
  Copyright:   E.S.R.F / CERN

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "FacetCoordinates.h"
#include "Facet.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"
#include <vector>

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

static const int   flWidth[] = {35,40,100,100,100};
static const char *flName[] = {"#","Vertex","X","Y","Z"};
static const int   flAligns[] = { ALIGN_CENTER,ALIGN_CENTER,ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };
static const int   fEdits[] = { 0,0,EDIT_NUMBER,EDIT_NUMBER,EDIT_NUMBER };

FacetCoordinates::FacetCoordinates():GLWindow() {

  int wD = 405;
  int hD = 400;
  SetIconfiable(TRUE);
  SetResizable(FALSE);

  GLTitledPanel *p = new GLTitledPanel("Insert / Remove vertex");
  p->SetBounds(5,hD-120,wD-10,70);
  Add(p);

  GLLabel *l1 = new GLLabel("Vertex Id to insert:");
  setBounds(p,l1,20,20,120,19);
  Add(l1);

  insertIdText = new GLTextField(0,"0");
  setBounds(p,insertIdText,135,20,40,19);
  Add(insertIdText);

  insertLastButton = new GLButton(0,"Insert as last vertex");
  setBounds(p,insertLastButton,5,45,120,19);
  Add(insertLastButton);

  insertBeforeButton = new GLButton(0,"Insert before sel. row");
  setBounds(p,insertBeforeButton,135,45,120,19);
  insertBeforeButton->SetEnabled(FALSE);
  Add(insertBeforeButton);

  removePosButton = new GLButton(0,"Remove selected row");;
  setBounds(p,removePosButton,265,45,120,19);
  removePosButton->SetEnabled(FALSE);
  Add(removePosButton);
  
  updateButton = new GLButton(0,"Apply");
  updateButton->SetBounds(wD-195,hD-43,90,19);
  Add(updateButton);

  setXbutton = new GLButton(0, "X");
  setXbutton->SetBounds(5, hD - 43, 16, 19);
  Add(setXbutton);
  setYbutton = new GLButton(0, "Y");
  setYbutton->SetBounds(27, hD - 43, 16, 19);
  Add(setYbutton);
  setZbutton = new GLButton(0, "Z");
  setZbutton->SetBounds(49, hD - 43, 16, 19);
  Add(setZbutton);

  dismissButton = new GLButton(0,"Dismiss");
  dismissButton->SetBounds(wD-95,hD-43,90,19);
  Add(dismissButton);

  facetListC = new GLList(0);
  facetListC->SetSize(5,1);
  facetListC->SetBounds(5,5,wD-10,hD-130);
  facetListC->SetColumnLabelVisible(TRUE);
  facetListC->SetGrid(TRUE);
  //facetListC->SetSelectionMode(SINGLE_CELL);
  //facetListC->Sortable=TRUE;
  Add(facetListC);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  RestoreDeviceObjects();

}

void FacetCoordinates::GetSelected() {

  selFacet = NULL;
  int i = 0;
  Geometry *g = worker->GetGeometry();
  int nb = g->GetNbFacet();
  while(!selFacet && i<nb) {
    if( g->GetFacet(i)->selected ) selFacet = g->GetFacet(i);
    if(!selFacet) i++;
  }

  char tmp[32];
  sprintf(tmp,"Facets coordinates #%d",i+1);
  SetTitle(tmp);

}

void FacetCoordinates::UpdateFromSelection() {

  
  if(!IsVisible()) return;
  GetSelected();
  if(!selFacet) return;

  Geometry *geom = worker->GetGeometry();

  int nbIndex = selFacet->sh.nbIndex;

  lines=std::vector<line>();

  for (size_t i=0;(int)i<nbIndex;i++) {
	  line newLine;
	  newLine.coord=*geom->GetVertex(newLine.vertexId=selFacet->indices[i]);
	  lines.push_back(newLine);
  }

  RebuildList();

}

void FacetCoordinates::Display(Worker *w) {

  worker = w;
  SetVisible(TRUE);
  UpdateFromSelection();

}

void FacetCoordinates::ProcessMessage(GLComponent *src,int message) {

  Geometry *geom = worker->GetGeometry();
  switch(message) {
    case MSG_BUTTON:
      if(src==dismissButton) {
        SetVisible(FALSE);
	  } else if (src==insertLastButton) {
		  int vertexId;
		  int rowId=facetListC->GetNbRow();
		  if (!(insertIdText->GetNumberInt(&vertexId)) || !(vertexId>=1 && vertexId<=geom->GetNbVertex())) {
			  GLMessageBox::Display("Wrong vertex Id entered","Wrong number",GLDLG_OK,GLDLG_ICONWARNING);
			  break;
		  }
		  InsertVertex(rowId,vertexId-1);
	  } else if (src==insertBeforeButton) {
		  int vertexId;
		  int rowId=facetListC->GetSelectedRow();
		  if (!(insertIdText->GetNumberInt(&vertexId)) || !(vertexId>=1 && vertexId<=geom->GetNbVertex())) {
			  GLMessageBox::Display("Wrong vertex Id entered","Wrong number",GLDLG_OK,GLDLG_ICONWARNING);
			  break;
		  }
		  InsertVertex(rowId,vertexId-1);
	  } else if (src==removePosButton) {
		  RemoveRow(facetListC->GetSelectedRow());
	  } else if(src==updateButton) {
		  ApplyChanges();
		  break;
	  }
	  else if (src == setXbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all X coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int row=0;row<(int)lines.size();row++) {
			  lines[row].coord.x=coordValue;
		  }
		  RebuildList();
	  }
	  else if (src == setYbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all Y coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int row = 0; row<(int)lines.size(); row++) {
			  lines[row].coord.y = coordValue;
		  }
		  RebuildList();
	  }
	  else if (src == setZbutton) {
		  double coordValue;
		  char *coord = GLInputBox::GetInput("0", "New coordinate:", "Set all Z coordinates to:");
		  if (!coord) return;
		  if (!sscanf(coord, "%lf", &coordValue)) {
			  GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
			  return;
		  }
		  for (int row = 0; row<(int)lines.size(); row++) {
			  lines[row].coord.z = coordValue;
		  }
		  RebuildList();
	  }
    case MSG_LIST:
      if(src==facetListC) {
        int selRow=facetListC->GetSelectedRow()+1;
		insertBeforeButton->SetEnabled(selRow);
		removePosButton->SetEnabled(selRow);
      }
    break;

  }

  GLWindow::ProcessMessage(src,message);

}

void FacetCoordinates::setBounds(GLComponent *org,GLComponent *src,int x,int y,int w,int h) {
  int xc,yc,wc,hc;
  org->GetBounds(&xc,&yc,&wc,&hc);
  src->SetBounds(xc+x,yc+y,w,h);
}

void FacetCoordinates::UpdateId(int vertexId) {
	char tmp[64];
	sprintf(tmp,"%d",vertexId+1);
	insertIdText->SetText(tmp);
}

void FacetCoordinates::RebuildList() {
	facetListC->SetSize(5,(int)lines.size());
	
	facetListC->SetColumnWidths((int*)flWidth);
	facetListC->SetColumnLabels((char **)flName);
	facetListC->SetColumnAligns((int *)flAligns);
	facetListC->SetColumnEditable((int *)fEdits);
	
	char tmp[128];

	for(int i=0;i<(int)lines.size();i++) {
		
		sprintf(tmp,"%d",i+1);
		facetListC->SetValueAt(0,i,tmp);
		
		sprintf(tmp,"%d",lines[i].vertexId+1);
		facetListC->SetValueAt(1,i,tmp);
    
		sprintf(tmp,"%.10g",lines[i].coord.x);
		facetListC->SetValueAt(2,i,tmp);

		sprintf(tmp,"%.10g",lines[i].coord.y);
		facetListC->SetValueAt(3,i,tmp);

		sprintf(tmp,"%.10g",lines[i].coord.z);
		facetListC->SetValueAt(4,i,tmp);
  }
	int selRow=facetListC->GetSelectedRow()+1;
		insertBeforeButton->SetEnabled(selRow);
		removePosButton->SetEnabled(selRow);
}

void FacetCoordinates::RemoveRow(int rowId){
 lines.erase(lines.begin()+rowId);
 RebuildList();
 if (rowId<facetListC->GetNbRow()) facetListC->SetSelectedRow(rowId);
 int selRow=facetListC->GetSelectedRow()+1;
 insertBeforeButton->SetEnabled(selRow);
 removePosButton->SetEnabled(selRow);
}

void FacetCoordinates::InsertVertex(int rowId,int vertexId){
	line newLine;
	newLine.vertexId=vertexId;
	newLine.coord=*(worker->GetGeometry()->GetVertex(vertexId));
	lines.insert(lines.begin()+rowId,newLine);
	RebuildList();

	facetListC->SetSelectedRow(rowId+1);
	int selRow=facetListC->GetSelectedRow()+1;
	insertBeforeButton->SetEnabled(selRow);
	removePosButton->SetEnabled(selRow);
	facetListC->ScrollToVisible(rowId,0);
}

void FacetCoordinates::ApplyChanges(){
	
	Geometry *geom = worker->GetGeometry();
	
	if (facetListC->GetNbRow()<3) {
		GLMessageBox::Display("A facet must have at least 3 vertices","Not enough vertices",GLDLG_OK,GLDLG_ICONWARNING);
		return;
	}
	
	//validate user inputs
	for (int row=0;row<(int)lines.size();row++) {
		double x,y,z;
		BOOL success = (1==sscanf(facetListC->GetValueAt(2,row),"%lf",&x));
		success = success && (1==sscanf(facetListC->GetValueAt(3,row),"%lf",&y));
		success = success && (1==sscanf(facetListC->GetValueAt(4,row),"%lf",&z));
		if (!success) { //wrong coordinates at row
				char tmp[128];
				sprintf(tmp,"Invalid coordinates in row %d",row+1);
				GLMessageBox::Display(tmp,"Incorrect vertex",GLDLG_OK,GLDLG_ICONWARNING);
				return;
		}
		lines[row].coord.x=x;
		lines[row].coord.y=y;
		lines[row].coord.z=z;
	}

	//int rep = GLMessageBox::Display("Apply geometry changes ?","Question",GLDLG_OK|GLDLG_CANCEL,GLDLG_ICONWARNING);
	//if( rep == GLDLG_OK ) {

		if (mApp->AskToReset(worker)) {
			mApp->changedSinceSave=TRUE;

			//Change number of vertices
			SAFE_FREE(selFacet->indices);
			SAFE_FREE(selFacet->vertices2);
			SAFE_FREE(selFacet->visible);
			selFacet->sh.nbIndex = (int)lines.size();
			selFacet->indices = (int *)malloc(selFacet->sh.nbIndex*sizeof(int));
			selFacet->vertices2 = (Vector2d *)malloc(selFacet->sh.nbIndex*sizeof(Vector2d));
			memset(selFacet->vertices2,0,selFacet->sh.nbIndex * sizeof(Vector2d));
			selFacet->visible = (BOOL *)malloc(selFacet->sh.nbIndex*sizeof(BOOL));
			memset(selFacet->visible,0xFF,selFacet->sh.nbIndex*sizeof(BOOL));

			for(size_t i=0;i<lines.size();i++) {
				geom->MoveVertexTo(lines[i].vertexId,lines[i].coord.x,lines[i].coord.y,lines[i].coord.z);
				selFacet->indices[i]=lines[i].vertexId;
			}
			geom->Rebuild(); //Will recalculate facet parameters

			// Send to sub process
			try {
				worker->Reload();
			} catch(Error &e) {
				GLMessageBox::Display((char *)e.GetMsg(),"Error",GLDLG_OK,GLDLG_ICONERROR);
				return;
			}
			//GLWindowManager::FullRepaint();
		}
	//}
}