/*
  File:        GLProgress.cpp
  Description: Simple Progress Dialog (SDL/OpenGL OpenGL application framework)
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

#include "GLProgress.h"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include "MathTools.h" //Min max Saturate
#include "GLWindowManager.h"

// Construct a message dialog box
GLProgress::GLProgress(char *message,char *title):GLWindow() {

  int xD,yD,wD,hD,txtWidth,txtHeight;
  int nbButton=0;
  lastUpd = 0;

  if(title) SetTitle(title);
  else      SetTitle("Message");

  // Label
  label = new GLLabel(message);
  label->GetTextBounds(&txtWidth,&txtHeight);
  txtWidth = MAX(txtWidth,150);
  label->SetBounds(67,(64-(txtHeight+22))/2,txtWidth,txtHeight);
  Add(label);

  // Scroll
  GLLabel *scrollBackground = new GLLabel("");
  scrollBackground->SetBorder(BORDER_BEVEL_IN);
  scrollBackground->SetBackgroundColor(200,200,200);
  scrollBackground->SetBounds(68,(64-(txtHeight+25))/2+22,150,20);
  Add(scrollBackground);

  scroll = new GLLabel("");
  scroll->SetBorder(BORDER_BEVEL_OUT);
  scroll->SetBackgroundColor(200,200,200);
  xP = 69;
  yP = (64-(txtHeight+25))/2+23;
  wP = 149;
  hP = 18;
  scroll->SetBounds(xP,yP,wP,hP);
  Add(scroll);

  scrollText = new GLLabel("100%");
  scrollText->SetOpaque(FALSE);
  scrollText->SetBounds(xP+wP/2-10,yP,20,hP);
  Add(scrollText);

  progress = 100;

  // Icon
  
  GLIcon   *gIcon = new GLIcon("images/icon_wait.png");
  gIcon->SetBounds(3,3,64,64);
  Add(gIcon);

  wD = txtWidth + 94;
  hD = MAX(txtHeight+35,90);

  // Center dialog
  int wS,hS;
  GLToolkit::GetScreenSize(&wS,&hS);
  if( wD > wS ) wD = wS;
  xD = (wS-wD)/2;
  yD = (hS-hD)/2;
  SetBounds(xD,yD,wD,hD);

  // Create objects
  RestoreDeviceObjects();

}

// -------------------------------------------------

void GLProgress::ProcessMessage(GLComponent *src,int message) {

  GLWindow::ProcessMessage(src,message);

}

// -------------------------------------------------

void GLProgress::SetProgress(double value) {

  Uint32 now = SDL_GetTicks();
  if((now-lastUpd) < 200 ) return;
  lastUpd = now;
  
  char tmp[128];
  int nW;
  double v = value;
  SATURATE(v,0.0,1.0);
  int p = (int)( v*100.0 + 0.5 );
  if( progress != p ) {
    progress = p;
    sprintf(tmp,"%d%%",progress);
    scrollText->SetText(tmp);
    nW = (int)((double)wP*v+0.5);
    scroll->SetBounds(xP,yP,nW,hP);
    GLWindowManager::Repaint();
	//this->Paint();
	//SDL_GL_SwapBuffers();
  }

}


double GLProgress::GetProgress() {

 return (double)progress/100.0;

}

void GLProgress::SetMessage(std::string msg) {
	SetMessage(msg.c_str());
}

void GLProgress::SetMessage(const char *msg) {

    label->SetText(msg);
	GLWindowManager::Repaint();
	//this->Paint();
	//SDL_GL_SwapBuffers();
}