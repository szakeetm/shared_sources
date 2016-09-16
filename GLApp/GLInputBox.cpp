/*
  File:        GLInputBox.cpp
  Description: Simple Input Dialog (SDL/OpenGL OpenGL application framework)
  Author:      J-L PONS (2007)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "GLInputBox.h"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLTextField.h"
#include "GLToolkit.h"

static char ret[1024];

// Construct a dialog box
GLInputBox::GLInputBox(char *message,char *label,char *title):GLWindow() {

  int wD;
  int hD=80;
  int txtWidth,txtHeight;

  if(title) SetTitle(title);
  else      SetTitle("Input");

  // Label
  GLLabel  *l1 = new GLLabel(label?label:"");
  l1->GetTextBounds(&txtWidth,&txtHeight);
  l1->SetBounds(5,5,txtWidth+5,19);
  Add(l1);
  wD = txtWidth + 100;

  //Text Field
  text = new GLTextField(0,message?message:"");
  text->SetEditable(TRUE);
  text->SetBounds(txtWidth+15,5,90,19);
  Add(text);

  // Buttons
  wD = MAX(txtWidth + 110,170);
  int startX = (wD-160)/2 + 3;
  int startY = hD - 43;

  GLButton *btn1 = new GLButton(1,"OK");
  btn1->SetBounds(startX,startY,75,20);
  Add(btn1);
  startX+=80;

  GLButton *btn2 = new GLButton(0,"Cancel");
  btn2->SetBounds(startX,startY,75,20);
  Add(btn2);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  int xD = (wS-wD)/2;
  int yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  // Create objects
  RestoreDeviceObjects();

  rValue = NULL;

}

// -------------------------------------------------

void GLInputBox::ProcessMessage(GLComponent *src,int message) {
  if(message==MSG_BUTTON) {
    if( src->GetId() ) {
      strcpy(ret,text->GetText());
      rValue = ret;
    }
	else {
      rValue = NULL;
    }
    GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    return;
  }
  else if (message==MSG_TEXT) {
		strcpy(ret,text->GetText());
		rValue = ret;
		GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    return;
	} 
  GLWindow::ProcessMessage(src,message);
}

// -------------------------------------------------

char *GLInputBox::GetInput(char *message,char *label,char *title) {

  GLfloat old_mView[16];
  GLfloat old_mProj[16];
  GLint   old_viewport[4];

  // Save current matrix
  glGetFloatv( GL_PROJECTION_MATRIX , old_mProj );
  glGetFloatv( GL_MODELVIEW_MATRIX , old_mView );
  glGetIntegerv( GL_VIEWPORT , old_viewport );

  // Initialise
  GLInputBox *dlg = new GLInputBox(message,label,title);
  dlg->UnfreezeComp();
  dlg->DoModal();
  char *ret = dlg->rValue;
  delete dlg;

  // Restore matrix
  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(old_mProj);
  glMatrixMode( GL_MODELVIEW );
  glLoadMatrixf(old_mView);
  glViewport(old_viewport[0],old_viewport[1],old_viewport[2],old_viewport[3]);

  return ret;

}
