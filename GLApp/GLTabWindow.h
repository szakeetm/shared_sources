/*
  File:        GLTabWindow.h
  Description: SDL/OpenGL OpenGL application framework
               Tabbed window component
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

#include "GLApp.h"
#include <malloc.h>

typedef struct {

  char         *name;
  int           width;
  GLComponent **comp;
  int           nbComp;
  BOOL          selected;

} APANEL;

class GLTabWindow : public GLWindow {

public:

  // Construction
  GLTabWindow();
  ~GLTabWindow();

  // Add/Remove components to this windows
  void Add(int panel,GLComponent *comp);
  void SetPanelNumber(int numP);
  void SetPanelName(int idx,char *name);
  void Clear();
  void Update();
  void SetTextColor(int r,int g,int b);

  //Overrides
  void SetBounds(int x,int y,int w,int h);
  void ProcessMessage(GLComponent *src,int message);

private:

  void showHide();

  APANEL *panels;
  int nbPanel;
  void *bar;

};

