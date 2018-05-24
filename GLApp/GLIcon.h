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
#ifndef _GLICONH_
#define _GLICONH_

#include "GLComponent.h"
#include "GLSprite.h"

class GLIcon : public GLComponent {

public:

  // Construction
  GLIcon(char *iconName);

  // Implementation
  void Paint();
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();

private:

  Sprite2D *icon;
  char name[512];

};

#endif /* _GLICONH_ */