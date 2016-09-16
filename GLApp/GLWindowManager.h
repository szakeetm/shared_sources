/*
  File:        GLWindowManager.h
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
#include <SDL.h>
#include <SDL_opengl.h>
#include "GLTypes.h"
#include "GLFont.h"

#ifndef _GLWINDOWMANAGERH_
#define _GLWINDOWMANAGERH_

#define NO_MODIFIER    0
#define ALT_MODIFIER   1
#define CTRL_MODIFIER  2
#define SHIFT_MODIFIER 4
#define CAPSLOCK_MODIFIER 8
#define SPACE_MODIFIER 16

class GLWindow;
class GLComponent;
class GLApplication;

class GLWindowManager {

public:

  // Window functions
  static void      Repaint();
  static void      RepaintNoSwap();
  static void      RepaintRange(int w0,int w1);
  static void      FullRepaint();
  static GLWindow* GetWindow(int idx);
  static int       GetNbWindow();
  static GLWindow *GetTopLevelWindow();
  static void      BringToFront(GLWindow *wnd);
  static void      SetDefault();

  // Key info
  static BOOL      IsCtrlDown();
  static BOOL      IsShiftDown();
  static BOOL      IsAltDown();
  static BOOL      IsCapsLockOn();
  static BOOL      IsSpaceDown();

  // Registering
  static void RegisterWindow(GLWindow *wnd);
  static void UnRegisterWindow(GLWindow *wnd);
  static void RegisterAcc(GLComponent *src,int keyCode,int modifier,int accId);

  // Processing
  static BOOL RestoreDeviceObjects(int width,int height);
  static BOOL ManageEvent(SDL_Event *evt);
  static BOOL ProcessKey(SDL_Event *evt,BOOL processAcc);
  static void RestoreDeviceObjects();
  static void InvalidateDeviceObjects();
  static void Resize();

  // Utils functions
  static void AnimateFocus(GLWindow *src);
  static void AnimateIconify(GLWindow *src);
  static void AnimateDeIconify(GLWindow *src);
  static void AnimateMaximize(GLWindow *src,int fsX,int fsY,int fsWidth,int fsHeight);
  static void RemoveAccFromStr(char *txt,char *acc=NULL,int *pos=NULL,int *width=NULL);
  static char *GetAccStr(int keyCode,int keyModifier);
  static void DrawStats();
  static void NoClip();

};

#endif /* _GLWINDOWMANAGERH_ */