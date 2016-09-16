/*
  File:        GLWindow.h
  Description: SDL/OpenGL OpenGL application framework (Window management)
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
#ifndef _GLWINDOWH_
#define _GLWINDOWH_

#include <SDL.h>
#include <SDL_opengl.h>
#include "GLContainer.h"
#include "GLFont.h"
#include "GLMenu.h"

class GLWindow : public GLContainer {

public:

  // Construction
  GLWindow();
  ~GLWindow();

  // Window methods
  void GetBounds(int *x,int *y,int *w,int *h);
  void SetPosition(int x,int y);
  int  GetHeight();
  int  GetWidth();
  void GetClientArea(int *x,int *y,int *w,int *h);
  void SetTitle(char *title);
  void SetBorder(BOOL b);
  void SetBackgroundColor(int r,int g,int b);
  void GetBackgroundColor(int *r,int *g,int *b);
  BOOL IsCtrlDown();
  BOOL IsShiftDown();
  BOOL IsAltDown();
  BOOL IsSpaceDown();
  BOOL IsCapsLockOn();
  int  GetX(GLComponent *src,SDL_Event *evt);
  int  GetY(GLComponent *src,SDL_Event *evt);
  int  GetScreenX(GLComponent *src);
  int  GetScreenY(GLComponent *src);
  void DoModal();
  void SetVisible(BOOL visible);
  BOOL IsVisible();
  void SetResizable(BOOL sizable);
  void SetIconfiable(BOOL iconifiable);
  void SetMinimumSize(int width,int height);
  void Iconify(BOOL iconify);
  void Maximise(BOOL max);
  BOOL IsIconic();
  BOOL IsMaximized();
  void SetAnimatedFocus(BOOL animate);

  // Expert usage
  void Clip(GLComponent *src,int lMargin,int uMargin,int rMargin,int bMargin);
  void ClipRect(GLComponent *src,int x,int y,int width,int height);
  void ClipToWindow();
  void ClipWindowExtent();
  BOOL IsMoving();
  BOOL IsInComp(GLComponent *src,int mx,int my);
  void SetMaster(BOOL master);
  BOOL IsDragging();
  int  GetIconWidth();
  void PaintTitle(int width,int height);
  void PaintMenuBar();
  void UpdateOnResize();

  // Menu management
  void SetMenuBar(GLComponent *bar,int hBar=20);
  void AddMenu(GLMenu *menu);
  void RemoveMenu(GLMenu *menu);
  void CloseMenu();

  //Implementation
  virtual void ProcessMessage(GLComponent *src,int message);
  virtual void ManageEvent(SDL_Event *evt);
  virtual void ManageMenu(SDL_Event *evt);
  virtual void Paint();
  virtual void PaintMenu();
  virtual void SetBounds(int x,int y,int w,int h);
  virtual void CancelDrag(SDL_Event *evt);

  

protected:

  // Coordinates (absolute)
  int  width;
  int  height;
  int  posX;
  int  posY;

private:

  int  GetUpMargin();
  BOOL IsInWindow(int mx,int my);
  BOOL IsInSysButton(SDL_Event *evt,int witch);
  void UpdateSize(int newWidht,int newHeight,int cursor);

  int  draggMode;
  int  mXOrg;
  int  mYOrg;
  char title[128];
  char iconTitle[64];
  int  closeState;
  int  maxState;
  int  iconState;
  BOOL iconifiable;
  BOOL iconified;
  BOOL maximized;
  BOOL border;
  BOOL animateFocus;
  int  rBack;
  int  gBack;
  int  bBack;
  BOOL isMaster;
  GLComponent *menuBar;
  BOOL visible;
  BOOL isResizable;
  int  minWidth;
  int  minHeight;
  int  orgWidth;
  int  orgHeight;
  GLContainer *menus;
  BOOL isModal;
  int  iconWidth;
  int  posXSave;
  int  posYSave;
  int  widthSave;
  int  heightSave;
  DWORD lastClick;

};

#endif /* _GLWINDOWH_ */