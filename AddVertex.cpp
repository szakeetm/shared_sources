/*
  File:        AddVertex.cpp
  Description: Add vertex dialog
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

#include "AddVertex.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLWindowManager.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp\GLTextField.h"
#include "GLApp\GLLabel.h"
#include "GLApp\GLButton.h"
#include "Geometry.h"

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

AddVertex::AddVertex(Geometry *g,Worker *w):GLWindow() {

  int wD = 200;
  int hD = 130;

  SetTitle("Add new vertex by coordinates");

  GLLabel *l1 = new GLLabel("X");
  l1->SetBounds(10,5,170,18);
  Add(l1);

  x = new GLTextField(0,"0");
  x->SetBounds(100,5,80,18);
  Add(x);

  GLLabel *l2 = new GLLabel("Y");
  l2->SetBounds(10,30,170,18);
  Add(l2);

  y = new GLTextField(0,"0");
  y->SetBounds(100,30,80,18);
  Add(y);
  
  GLLabel *l3 = new GLLabel("Z");
  l3->SetBounds(10,55,170,18);
  Add(l3);

  z = new GLTextField(0,"0");
  z->SetBounds(100,55,80,18);
  Add(z);

  addButton = new GLButton(0,"Add");
  addButton->SetBounds(10,hD-44,85,21);
  Add(addButton);

  cancelButton = new GLButton(0,"Dismiss");
  cancelButton->SetBounds(wD-90,hD-44,85,21);
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



void AddVertex::ProcessMessage(GLComponent *src,int message) {

  double X,Y,Z;

  switch(message) {
    case MSG_BUTTON:

    if(src==cancelButton) {

      GLWindow::ProcessMessage(NULL,MSG_CLOSE);

    } else if (src==addButton) {

      if( !x->GetNumber(&X) ) {
        GLMessageBox::Display("Invalid X coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
        return;
      }
	        if( !y->GetNumber(&Y) ) {
        GLMessageBox::Display("Invalid Y coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
        return;
      }
			      if( !z->GetNumber(&Z) ) {
        GLMessageBox::Display("Invalid Z coordinate","Error",GLDLG_OK,GLDLG_ICONERROR);
        return;
      }
	  //if (work->running) work->Stop_Public();
		  geom->AddVertex(X,Y,Z);
	  //work->Reload();
	  //mApp->changedSinceSave = true; //Adding a vertex doesn't modify the simulation whatsoever


      //GLWindowManager::FullRepaint();

    }
    break;
  }

  GLWindow::ProcessMessage(src,message);
}

