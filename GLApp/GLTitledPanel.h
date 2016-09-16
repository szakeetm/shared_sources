/*
  File:        GLTitledPanel.h
  Description: Titled panel class (SDL/OpenGL OpenGL application framework)
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

#ifndef _GLTITLEDPANELH_
#define _GLTITLEDPANELH_

class GLTitledPanel : public GLComponent {

public:

  // Construction
  GLTitledPanel(char *title);

  // Component method
  void SetTitle(char *title);
  void SetTextColor(int r,int g,int b);
  void SetBold(BOOL b);
  void SetClosable(BOOL c);
  void Close();
  void Open();
  BOOL IsClosed();
  void SetCompBounds(GLComponent *src,int x,int y,int width,int height);

  // Implementation
  virtual void Paint();
  virtual void ManageEvent(SDL_Event *evt);
  virtual void ProcessMessage(GLComponent *src,int message);
  virtual void SetBounds(int x,int y,int width,int height);

private:

  char  title[256];
  float rText;
  float gText;
  float bText;
  BOOL  isBold;
  BOOL  closeAble;
  BOOL  closeState;
  BOOL  closed;
  int   wOrg;
  int   hOrg;
  int   txtWidth;

};

#endif /* _GLTITLEDPANELH_ */