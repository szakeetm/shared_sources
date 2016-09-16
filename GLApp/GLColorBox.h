/*
  File:        GLColorBox.h
  Description: Simple color chooser dialog (SDL/OpenGL OpenGL application framework)
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
#include "GLWindow.h"
#include "GLLabel.h"
#include "GLTitledPanel.h"
#include "GLTextField.h"

#ifndef _GLCOLORBOXH_
#define _GLCOLORBOXH_

class GLColorBox : private GLWindow {

public:
  // Display a modal dialog and return 1 on ok, 0 on cancel. r,g and b 
  // contains the choosen color
  static int Display(char *title,int *r,int *g,int *b);

  int  rCode;
  int  curR;
  int  curG;
  int  curB;

  // Implementation
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();

private:

  GLTitledPanel *swPanel;
  GLTitledPanel *hsvPanel;
  GLTextField   *rText;
  GLTextField   *gText;
  GLTextField   *bText;
  GLLabel       *oldColor;
  GLLabel       *newColor;
  GLLabel       *swBox;
  GLLabel       *hsBox;
  GLLabel       *vBox;
  GLuint         hsvTex;
  GLuint         sliderTex;
  float          curH;
  float          curS;
  float          curV;
  BOOL           draggV;

  GLColorBox(char *title,int *r,int *g,int *b);
  ~GLColorBox();
  void ProcessMessage(GLComponent *src,int message);
  void Paint();
  void ManageEvent(SDL_Event *evt);

  void rgb_to_hsv( int ri,int gi,int bi, float *h,float *s,float *v);
  DWORD hsv_to_rgb( float h,float s,float v,BOOL swap=FALSE );
  float get_red( DWORD c );
  float get_green( DWORD c );
  float get_blue( DWORD c );
  int   get_redi( DWORD c );
  int   get_greeni( DWORD c );
  int   get_bluei( DWORD c );
  void  paintBox(int x,int y,int w,int h);
  void  updateColor(int r,int g,int b);

};

#endif /* _GLCOLORBOXH_ */
