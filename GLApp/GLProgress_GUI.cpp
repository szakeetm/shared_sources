// Copyright (c) 2011 rubicon IT GmbH
#include "GLProgress_GUI.hpp"
#include "GLButton.h"
#include "GLLabel.h"
#include "GLIcon.h"
#include "GLToolkit.h"
#include "Helper/MathTools.h" //Min max Saturate
#include "GLWindowManager.h"
#include <fmt/core.h>

// Construct a message dialog box
GLProgress_GUI::GLProgress_GUI(const std::string &message,const std::string &title) : GLProgress_Abstract(message)  {

  lastUpd = 0;

  if(!title.empty()) SetTitle(title);
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

  percentLabel = new GLLabel("0%");
  percentLabel->SetOpaque(false);
  Add(percentLabel);

  // Icon
  
  GLIcon   *gIcon = new GLIcon("images/icon_wait.png");
  gIcon->SetBounds(3,3,64,64);
  Add(gIcon);

  //Fixed values
  progressBarX = 69;
  progressBarWidth = 0; //0%
  progressBarMaxWidth = 249;
  progressBarHeight = 18;

  PositionComponents(); //Dynamic values depending on text width, height

  // Create objects
  RestoreDeviceObjects();

}

void GLProgress_GUI::ProcessMessage(GLComponent *src,int message) {

  GLWindow::ProcessMessage(src,message);

}

void GLProgress_GUI::SetProgress(const double value) {
	double myVal = value;
  Saturate(myVal,0.0,1.0);
  int p = (int)(myVal *100.0 + 0.5 );
  if( progress != p ) {
    progress = p;

    percentLabel->SetText(fmt::format("{}%",progress));
	progressBarWidth = (int)((double)progressBarMaxWidth* myVal +0.5);
    progressBar->SetBounds(progressBarX,progressBarY,progressBarWidth,progressBarHeight); //lighter than PositionComponents()

	Uint32 now = SDL_GetTicks();
	if (IsVisible() && (now - lastUpd) > 500) {
		GLWindowManager::Repaint();
		lastUpd = now;
	}
  }

}

void GLProgress_GUI::SetMessage(const std::string& msg, const bool newLine, const bool forceDraw) {
	//The newLine is unused in GUI, required to override abstract progress bar
	status = msg;
	progressStatus->SetText(msg);
	PositionComponents(); //If message length or height changed
	Uint32 now = SDL_GetTicks();
	if (IsVisible() && forceDraw || ((now - lastUpd) > 500)) {
		GLWindowManager::Repaint();
		lastUpd = now;
	}
}

void GLProgress_GUI::PositionComponents()
{
	int txtWidth, txtHeight;
	progressStatus->GetTextBounds(&txtWidth, &txtHeight);
	txtWidth = std::max(txtWidth, 250); //For short status, minimum 250
	progressStatus->SetBounds(67, 14, txtWidth, txtHeight);

	progressBar->SetBounds(68, 20 + txtHeight, progressBarMaxWidth, 20);
	progressBarY = 21 + txtHeight;

	progressBarBackground->SetBounds(progressBarX-1, progressBarY-1, progressBarMaxWidth+1, progressBarHeight+2);
	progressBar->SetBounds(progressBarX, progressBarY, progressBarWidth, progressBarHeight);
	
	percentLabel->SetBounds(progressBarX + progressBarMaxWidth / 2 - 10, progressBarY, 20, progressBarHeight);

	int windowWidth = txtWidth + 94;
	int windowHeight = std::max(txtHeight + 70, 90);

	// Center dialog
	int screenWidth, screenHeight;
	GLToolkit::GetScreenSize(&screenWidth, &screenHeight);
	if (windowWidth > screenWidth) windowWidth = screenWidth;
	int windowX = (screenWidth - windowWidth) / 2 - 30;
	int windowY = (screenHeight - windowHeight) / 2;
	SetBounds(windowX, windowY, windowWidth, windowHeight);
}