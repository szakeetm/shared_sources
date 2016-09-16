/*
  File:        GLLabel.h
  Description: Label class (SDL/OpenGL OpenGL application framework)
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
#include "GLFont.h"

#ifndef _GLLABELH_
#define _GLLABELH_

class GLLabel : public GLComponent {

public:

  // Construction
  GLLabel(const char *text);
  ~GLLabel();

  // Component method
  void SetText(const char *text);
  void SetTextColor(int r,int g,int b);
  void GetTextBounds(int *w,int *h);
  void Clear();
  char* GetText();

  // Implementation
  void Paint();
  void RestoreDeviceObjects();

protected:

  char *mText;
  char *lines[64];
  int  nbLine;
  int  txtWidth;
  int  txtHeight;
  float rText;
  float gText;
  float bText;
  GLFont2D *font; 

};

class GLOverlayLabel : public GLLabel {
public:
	GLOverlayLabel(char *text);
	void SetBackgroundColor(float r,float g,float b);
	void Paint();
	void SetTextSize(float s);
	BOOL paintBg;
private:
	float rBack;
	float gBack;
	float bBack;
	float sizeFactor;
};
#endif /* _GLLABELH_ */