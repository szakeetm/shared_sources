/*
  File:        MoveVertex.cpp
  Description: Move vertex by offset dialog
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

#include "MoveVertex.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"

MoveVertex::MoveVertex(Geometry *g,Worker *w):GLWindow() {

  int wD = 300;
  int hD = 130;

  SetTitle("Move selected vertices by offset");

  GLLabel *l1 = new GLLabel("dX");
  l1->SetBounds(10,5,170,18);
  Add(l1);

  xOffset = new GLTextField(0,"0");
  xOffset->SetBounds(100,5,80,18);
  Add(xOffset);

  GLLabel *l2 = new GLLabel("dY");
  l2->SetBounds(10,30,170,18);
  Add(l2);

  yOffset = new GLTextField(0,"0");
  yOffset->SetBounds(100,30,80,18);
  Add(yOffset);
  
  GLLabel *l3 = new GLLabel("dZ");
  l3->SetBounds(10,55,170,18);
  Add(l3);

  zOffset = new GLTextField(0,"0");
  zOffset->SetBounds(100,55,80,18);
  Add(zOffset);

  moveButton = new GLButton(0,"Move");
  moveButton->SetBounds(10,hD-44,85,21);
  Add(moveButton);

  copyButton = new GLButton(0,"Copy");
  copyButton->SetBounds(100,hD-44,85,21);
  Add(copyButton);

  cancelButton = new GLButton(0,"Dismiss");
  cancelButton->SetBounds(190,hD-44,85,21);
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



void MoveVertex::ProcessMessage(GLComponent *src,int message) {

  double dX,dY,dZ;

  switch(message) {
    case MSG_BUTTON:

    if(src==cancelButton) {

      GLWindow::ProcessMessage(NULL,MSG_CLOSE);

    } else if (src==moveButton || src==copyButton) {
				if (geom->GetNbSelectedVertex()==0) {
			GLMessageBox::Display("No vertex selected","Nothing to move",GLDLG_OK,GLDLG_ICONERROR);
			return;
		}

      if( !xOffset->GetNumber(&dX) ) {
        GLMessageBox::Display("Invalid X offset value","Error",GLDLG_OK,GLDLG_ICONERROR);
        return;
      }
	        if( !yOffset->GetNumber(&dY) ) {
        GLMessageBox::Display("Invalid Y offset value","Error",GLDLG_OK,GLDLG_ICONERROR);
        return;
      }
			      if( !zOffset->GetNumber(&dZ) ) {
        GLMessageBox::Display("Invalid Z offset value","Error",GLDLG_OK,GLDLG_ICONERROR);
        return;
      }
		if (work->running) work->Stop_Public();
      
			geom->MoveSelectedVertex(dX,dY,dZ,src==copyButton,work);
			try { work->Reload(); } catch(Error &e) {
					GLMessageBox::Display((char *)e.GetMsg(),"Error reloading worker",GLDLG_OK,GLDLG_ICONERROR);
			}
      //GLWindowManager::FullRepaint();

    }
    break;
  }

  GLWindow::ProcessMessage(src,message);
}

