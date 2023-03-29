// Copyright (c) 2011 rubicon IT GmbH
#include "GLProgress.h"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include "Helper/MathTools.h" //Min max Saturate
#include "GLWindowManager.h"
#include <sstream>

// Construct a message dialog box
GLProgress::GLProgress(const char *message,const char *title):GLWindow() {


  lastUpd = 0;

  if(title) SetTitle(title);
  else      SetTitle("Message");

  // Label
  progressStatus = new GLLabel(message);

  Add(progressStatus);

  // Scroll
  progressBarBackground = new GLLabel("");
  progressBarBackground->SetBorder(BORDER_BEVEL_IN);
  progressBarBackground->SetBackgroundColor(200,200,200);
  Add(progressBarBackground);

  progressBar = new GLLabel("");
  progressBar->SetBorder(BORDER_BEVEL_OUT);
  progressBar->SetBackgroundColor(200,200,200);
  
  Add(progressBar);

  percentLabel = new GLLabel("100%");
  percentLabel->SetOpaque(false);
  Add(percentLabel);

  progress = 100;

  // Icon
  
  GLIcon   *gIcon = new GLIcon("images/icon_wait.png");
  gIcon->SetBounds(3,3,64,64);
  Add(gIcon);

  //Fixed values
  progressBarX = 69;
  progressBarWidth = progressBarMaxWidth = 249;
  progressBarHeight = 18;

  PositionComponents(); //Dynamic values depending on text width, height

  // Create objects
  RestoreDeviceObjects();

}

void GLProgress::ProcessMessage(GLComponent *src,int message) {

  GLWindow::ProcessMessage(src,message);

}

void GLProgress::SetProgress(double value) {
  
  Saturate(value,0.0,1.0);
  size_t p = (size_t)( value*100.0 + 0.5 );
  if( progress != p ) {
    progress = p;
	std::ostringstream pct;
	pct << progress << "%";
    percentLabel->SetText(pct.str());
	progressBarWidth = (int)((double)progressBarMaxWidth*value+0.5);
    progressBar->SetBounds(progressBarX,progressBarY,progressBarWidth,progressBarHeight); //lighter than PositionComponents()

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

void GLProgress::SetMessage(const std::string& msg, const bool& force) {
	SetMessage(msg.c_str(),force);
}

void GLProgress::PositionComponents()
{
	int txtWidth, txtHeight;
	progressStatus->GetTextBounds(&txtWidth, &txtHeight);
	txtWidth = Max(txtWidth, 250); //For short status, minimum 150
	progressStatus->SetBounds(67, 14, txtWidth, txtHeight);

	progressBar->SetBounds(68, 20 + txtHeight, progressBarMaxWidth, 20);
	progressBarY = 21 + txtHeight;

	progressBarBackground->SetBounds(progressBarX-1, progressBarY-1, progressBarMaxWidth+1, progressBarHeight+2);
	progressBar->SetBounds(progressBarX, progressBarY, progressBarWidth, progressBarHeight);
	
	percentLabel->SetBounds(progressBarX + progressBarMaxWidth / 2 - 10, progressBarY, 20, progressBarHeight);

	int windowWidth = txtWidth + 94;
	int windowHeight = Max(txtHeight + 70, 90);

	// Center dialog
	int screenWidth, screenHeight;
	GLToolkit::GetScreenSize(&screenWidth, &screenHeight);
	if (windowWidth > screenWidth) windowWidth = screenWidth;
	int windowX = (screenWidth - windowWidth) / 2 - 30;
	int windowY = (screenHeight - windowHeight) / 2;
	SetBounds(windowX, windowY, windowWidth, windowHeight);
}

void GLProgress::SetMessage(const char *msg, const bool& force) {

    progressStatus->SetText(msg);
	PositionComponents(); //If message length or height changed
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