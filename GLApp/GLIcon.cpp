/*
  File:        GLIcon.cpp
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
#include "GLWindow.h"
#include "GLIcon.h"
#include "GLToolkit.h"

// ---------------------------------------------------------------------

GLIcon::GLIcon(char *name):GLComponent(0) {
  strcpy(this->name,name);
  icon = NULL;
}

// ---------------------------------------------------------------------

void GLIcon::Paint() {

  if(!parent) return;

  if(icon) {
    icon->UpdateSprite(posX,posY,posX+width,posY+height);
    icon->Render(FALSE);
  }

}

// ---------------------------------------------------------------------

void GLIcon::InvalidateDeviceObjects() {
  icon->InvalidateDeviceObjects();
  SAFE_DELETE(icon);
}

// ----------------------------------------------------------

void GLIcon::RestoreDeviceObjects() {

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT,viewport);
  icon = new Sprite2D();
  icon->RestoreDeviceObjects(name,"none",viewport[2],viewport[3]);
  icon->SetSpriteMapping(0.0f,0.0f,1.0f,1.0f);

}
