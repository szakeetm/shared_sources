/*
  File:        GLComponent.h
  Description: Base component class (SDL/OpenGL OpenGL application framework)
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
#ifndef _GLCOMPONENTH_
#define _GLCOMPONENTH_

#include <SDL.h>
#include <SDL_opengl.h>
#include "GLTypes.h"
#include "GLContainer.h"

#define BORDER_NONE      0
#define BORDER_BEVEL_IN  1
#define BORDER_BEVEL_OUT 2
#define BORDER_ETCHED    3

class GLComponent: public GLContainer {

public:

  // Construction
  GLComponent(int compId);

  // Component methods
  void SetPosition(int x,int y);
  void SetSize(int width,int height);
  void GetBounds(int *x,int *y,int *w,int *h);
  void SetBorder(int border);
  void SetOpaque(BOOL opaque);
  void SetBackgroundColor(int r,int g,int b);
  void GetBackgroundColor(int *r,int *g,int *b);
  int  GetId();
  BOOL HasFocus();
  void SetFocusable(BOOL acceptFocus);
  BOOL IsFocusable();
  void SetEnabled(BOOL enable);
  BOOL IsEnabled();
  void SetVisible(BOOL visible);
  BOOL IsVisible();
  GLContainer *GetParent();
  void SetCursor(int cursor);
  int GetCursor();
  int GetWidth();
  int GetHeight();

  // Implementation
  virtual void Paint();
  virtual void SetFocus(BOOL focus);
  virtual void SetBounds(int x,int y,int width,int height);
  virtual void SetParent(GLContainer *parent);

protected:

  int  width;
  int  height;
  int  posX;
  int  posY;
  BOOL opaque;
  int  border;
  int  rBack;
  int  gBack;
  int  bBack;
  int  id;
  int  cursor;
  BOOL focus;
  BOOL enabled;
  BOOL visible;
  BOOL focusable;
  GLContainer *parent;

};

#endif /* _GLCOMPONENTH_ */