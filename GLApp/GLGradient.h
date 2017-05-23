/*
  File:        GLGradient.h
  Description: Gradient viewer class (SDL/OpenGL OpenGL application framework)
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

#ifndef _GLGRADIENTH_
#define _GLGRADIENTH_

#include "GLComponent.h"
#include "GLChart\GLChartConst.h" //linear, log scale constants
class GLAxis;
class GLLabel;

#define GRADIENT_BW    0
#define GRADIENT_COLOR 1

class GLGradient : public GLComponent {

public:

  // Construction
  GLGradient(int compId);
  ~GLGradient();

  // Component methods
  void SetMinMax(double min,double max);
  void SetType(int type);
  void SetScale(int scale);
  int  GetScale();
  void SetMouseCursor(bool enable);

  // Implementation
  void Paint();
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();
  void ManageEvent(SDL_Event *evt);

private:

  void UpdateValue();

  GLuint       colorTex;
  GLuint       bwTex;
  GLAxis       *axis;
  GLLabel      *mouseValue;
  bool         mouseCursor;
  int          gType;

  int          gWidth;
  int          gHeight;
  int          gPosX;
  int          gPosY;
  double       cursorPos;

};

#endif /* _GLGRADIENTH_ */