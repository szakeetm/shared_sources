/*
  File:        GLToggle.h
  Description: Toggle Button class (SDL/OpenGL OpenGL application framework)
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
#include <string>

#ifndef _GLTOGGLEH_
#define _GLTOGGLEH_

class GLToggle : public GLComponent {

public:

  // Construction
  GLToggle(int compId,char *text);

  // Component method
  int  GetState();
  void SetText(std::string txt);
  void SetState(int setState);
  void SetTextColor(int r,int g,int b);
  void SetEnabled(BOOL enable); //override GLComponent for text color change
  void AllowMixedState(BOOL setAllow);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  char text[512];
  int  state; // 0=>Unchekced 1=>Checked 2=>Multiple
  float rText;
  float gText;
  float bText;
  BOOL allowMixedState; //Allow "multiple" state

};

#endif /* _GLTOGGLEH_ */