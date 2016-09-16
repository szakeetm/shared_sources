/*
   File:        GLSprite.h
  Description: 2D Sprites (SDL/OpenGL OpenGL application framework)
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

#include <SDL_opengl.h>

#ifndef _SPRITE2DH_
#define _SPRITE2DH_

class Sprite2D {

public:

  // Default constructor
  Sprite2D();
  
  // Initialise the font
  // return 1 when success, 0 otherwise
  int RestoreDeviceObjects(char *diffName,char *alphaName,int srcWidth,int scrHeight);

  // Update sprite mapping and coordinates
  void UpdateSprite(const int &x1,const int &y1,const int &x2,const int &y2);
  void UpdateSprite(const int &x1,const int &y1,const int &x2,
	  const int &y2,const float &mx1,const float &my1,const float &mx2,const float &my2);
  void SetSpriteMapping(const float &mx1,const float &my1,const float &mx2,const float &my2);
  void SetColor(const float &r,const float &g,const float &b);
  
  // Draw a 2D sprite (in screen coordinates)
  void Render(const BOOL &doLinear);
  void Render90(BOOL doLinear);

  // Release any allocated resource
  void InvalidateDeviceObjects();

private:

  GLuint  texId;
  int x1;
  int y1;
  int x2;
  int y2;
  float mx1;
  float my1;
  float mx2;
  float my2;
  int hasAlpha;
  float rC;
  float gC;
  float bC;

  GLfloat pMatrix[16];

};

#endif /* _SPRITE2DH_ */
