/*
  File:        GLContainer.h
  Description: Base component container class (SDL/OpenGL OpenGL application framework)
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

#ifndef _GLCONTAINERH_
#define _GLCONTAINERH_

class GLComponent;
class GLWindow;

typedef struct COMPLINK {
  GLComponent *comp;
  BOOL postDelete;
  BOOL canProcess;
  COMPLINK *next;
} COMP_LIST;

class GLContainer {

public:

  // Construction
  GLContainer();
  ~GLContainer();

  // Parent window
  void SetWindow(GLWindow *parent);
  GLWindow *GetWindow();

  // Components method
  void Add(GLComponent *comp);
  void Remove(GLComponent *comp);
  void SetFocus(GLComponent *src);
  void PaintComponents();
  void DoPostDelete();
  void PostDelete(GLComponent *comp);
  void Clear();
  GLComponent* GetFirstChildComp();

  // Event stuff
  void FreezeComp();
  void UnfreezeComp();
  BOOL IsEventProcessed();
  BOOL IsEventCanceled();
  void RelayEvent(SDL_Event *evt);
  void RedirectMessage(GLContainer *cont);
  GLContainer *GetRedirect();

  // Implementation
  virtual void ManageEvent(SDL_Event *evt);
  virtual void InvalidateDeviceObjects();
  virtual void RestoreDeviceObjects();
  virtual void ProcessMessage(GLComponent *src,int message);
  virtual void ProcessAcc(int accId);
  virtual void CancelDrag(SDL_Event *evt);
  virtual BOOL IsDragging();

protected:

  void RelayEvent(GLComponent *comp,SDL_Event *evt,int ox=0,int oy=0);
  void RelayEventReverse(COMPLINK *lst,SDL_Event *evt);
  void ManageComp(GLComponent *comp,SDL_Event *evt);

  GLComponent *lastFocus;
  GLComponent *draggedComp;
  BOOL evtProcessed;
  BOOL evtCanceled;

private:

  COMP_LIST *list;
  int  lastClick;
  GLWindow *parentWin;
  GLContainer *redirect;

};

#endif /* _GLCOMPONENTH_ */