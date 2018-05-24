/*
Program:     MolFlow+
Description: Monte Carlo simulator for ultra-high vacuum
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef _GLPROGESSH_
#define _GLPROGESSH_

//#include <SDL_opengl.h>
#include "GLWindow.h"
#include <string>

class GLLabel;

class GLProgress : public GLWindow {

public:

  GLProgress(char *message,char *title);

  // Update progress bar (0 to 1)
  void SetProgress(double value);
  double GetProgress();
  void SetMessage(const char *msg);
  void SetMessage(std::string msg);

private:

  GLLabel   *scroll;
  GLLabel   *scrollText;
  GLLabel	*label;
  int        progress;
  int        xP,yP,wP,hP;
  Uint32     lastUpd;

  void ProcessMessage(GLComponent *src,int message);

};

#endif /* _GLPROGESSH_ */
