// Copyright (c) 2011 rubicon IT GmbH
#include "GLProgress.h"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include "Helper/MathTools.h" //Min max Saturate
#include "GLWindowManager.h"

// Construct a message dialog box
GLProgress::GLProgress(const char *message,const char *title):GLWindow() {

  int xD,yD,wD,hD,txtWidth,txtHeight;
  int nbButton=0;
  lastUpd = 0;

  if(title) SetTitle(title);
  else      SetTitle("Message");
  SetClosable(false); //Progress bar can't be closed as it would hide an operation in progress

  // Label
  label = new GLLabel(message);
  label->GetTextBounds(&txtWidth,&txtHeight);
  txtWidth = Max(txtWidth,150);
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
  scrollText->SetOpaque(false);
  scrollText->SetBounds(xP+wP/2-10,yP,20,hP);
  Add(scrollText);

  progress = 100;

  // Icon
  
  GLIcon   *gIcon = new GLIcon("images/icon_wait.png");
  gIcon->SetBounds(3,3,64,64);
  Add(gIcon);

  wD = txtWidth + 94;
  hD = Max(txtHeight+35,90);

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

void GLProgress::ProcessMessage(GLComponent *src,int message) {

  GLWindow::ProcessMessage(src,message);

}

void GLProgress::SetProgress(double value) {
  
  char tmp[128];
  int nW;
  double v = value;
  Saturate(v,0.0,1.0);
  int p = (int)( v*100.0 + 0.5 );
  if( progress != p ) {
    progress = p;
    sprintf(tmp,"%d%%",progress);
    scrollText->SetText(tmp);
    nW = (int)((double)wP*v+0.5);
    scroll->SetBounds(xP,yP,nW,hP);

	Uint32 now = SDL_GetTicks();
	if (IsVisible() && (now - lastUpd) > 500) {
		GLWindowManager::Repaint();
		lastUpd = now;
	}
	//this->Paint();
	//SDL_GL_SwapBuffers();
  }

}

double GLProgress::GetProgress() {

 return (double)progress/100.0;

}

void GLProgress::SetMessage(std::string msg, const bool& force) {
	SetMessage(msg.c_str(),force);
}

void GLProgress::SetMessage(const char *msg, const bool& force) {

    label->SetText(msg);
	if (force) {
		GLWindowManager::Repaint();
	}
	else {
		Uint32 now = SDL_GetTicks();
		if (IsVisible() && (now - lastUpd) > 500) {
			GLWindowManager::Repaint();
			lastUpd = now;
		}
	}
	//this->Paint();
	//SDL_GL_SwapBuffers();
}