/*
   File:        GLFont.h
  Description: Simple 2D bitmap font (SDL/OpenGL OpenGL application framework)
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
#ifndef _GLFONT2DH_
#define _GLFONT2DH_

#include <SDL_opengl.h>
#include "GLTypes.h"

class GLFont2D {

public:

  // Default constructor
  GLFont2D();

  // Construct a font using the specified bitmap
  GLFont2D(char *imgFileName);
  
  // Initialise the font
  // return 1 when success, 0 otherwise
  int RestoreDeviceObjects(int srcWidth,int scrHeight);
  
  // Draw a 2D text (in viewport coordinates)
  void DrawText(const int &x,const int &y,char *text,const bool &loadMatrix=true);
  void DrawLargeText(int x,int y,char *text,float sizeFactor,bool loadMatrix=true);
  void DrawTextFast(int cx,int cy,const char *text);
  void DrawTextV(int x,int y,char *text,bool loadMatrix=true);

  // Release any allocated resource
  void InvalidateDeviceObjects();

  // Set text color
  void SetTextColor(const float &r,const float &g,const float &b);

  // Set default character size (Default 9,15)
  void SetTextSize(int width,int height);

  // Set variable font width (Must be called before RestoreDeviceObject)
  void SetVariableWidth(bool variable);

  // Get string size
  int GetTextWidth(const char *text);
  int GetTextHeight();

  // Adapat orthographic projection on viewport change
  void ChangeViewport(GLVIEWPORT *g);

private:

  char    fileName[512];
  GLuint  texId;
  int     fWidth;
  int     fHeight;
  int     cWidth;
  int     cHeight;
  float   rC;
  float   gC;
  float   bC;
  GLfloat pMatrix[16];
  int     cVarWidth[256];
  bool    isVariable;

};

#endif /* _GLFONT2DH_ */
