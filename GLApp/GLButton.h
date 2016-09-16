/*
  File:        GLButton.h
  Description: Button class (SDL/OpenGL OpenGL application framework)
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

#ifndef _GLBUTTONH_
#define _GLBUTTONH_

class GLButton : public GLComponent {

public:

  // Construction
  GLButton(int compId,char *text);

  // Components method
  void SetText(char *text);
  void SetIcon(char *fileName);
  void SetDisabledIcon(char *fileName);
  void SetToggle(BOOL toggle);
  BOOL GetState();
  void SetState(BOOL checked);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();
  void SetFontColor(float red, float green, float blue);
  void SetFontColor(int red, int green, int blue);
private:

  char text[256];
  Sprite2D *icon;
  Sprite2D *iconD;
  int  state; // 0=>Released 1=>Pressed
  char iconName[256];
  char iconNameDisa[256];
  BOOL toggle;
  BOOL toggleState;
  GLFont2D *font;
  float r, g, b;
};

#endif /* _GLBUTTONH_ */