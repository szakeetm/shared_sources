/*
  File:        GLIcon.h
  Description: Icon class (SDL/OpenGL OpenGL application framework)
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
#include "GLComponent.h"
#include "GLSprite.h"

#ifndef _GLICONH_
#define _GLICONH_

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