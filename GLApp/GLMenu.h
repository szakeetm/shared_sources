/*
  File:        GLMenu.h
  Description: Menu class (SDL/OpenGL OpenGL application framework)
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

#ifndef _GLMENUH_
#define _GLMENUH_

#define MAX_MENU_ITEM 64
#define MAX_ITEM_LGTH 128

class GLMenu;
class GLMenuBar;

// Menu item struct ----------------------------

typedef struct {

  char         itemName[MAX_ITEM_LGTH];
  int          itemId;
  int          x;
  int          y;
  int          width;
  int          height;
  BOOL         isSeparator;
  BOOL         checked;
  BOOL         enabled;
  char         shortcut;
  int          sctPos;
  int          sctWidth;
  GLMenu      *subMenu;
  char        *accName;
  int          accWidth;
  int          iconX;
  int          iconY;

} MENUITEM;

// Menu Popup ---------------------------------

class GLMenu : public GLComponent {

public:

  // Construction
  GLMenu();
  ~GLMenu();

  // Menu items
  void    Add(char *itemName,int itemId=0,int accKeyCode=0,int accKeyModifier=0);
  int     GetNbItem();
  GLMenu *GetSubMenu(char *itemName);
  void    SetState(int itemId,BOOL checked);
  BOOL    GetCheck(int itemId);
  void    SetEnabled(int itemId,BOOL enabled);
  void    SetIcon(int itemId,int x,int y);
  void    Clear();

  // Components method
  void    Close();
  void    Drop(GLContainer *parent,int x,int y);
  int     Track(GLWindow *parent,int x,int y);
  void    SetParentMenuBar(GLMenuBar *p);
  void    SetParentMenu(GLMenu *p);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void ProcessAcc(int accId);

private:

  BOOL  IsInItem(MENUITEM *p,int mx,int my);
  int   GetMenu(int mx,int my);
  int   GetMenu(int id);
  void  CloseSub(BOOL resetSel=TRUE);
  void  DropSub(int m);
  BOOL  HasSub(int s);
  void  ProcessMenuItem(int m);
  BOOL  ProcessShortcut(SDL_Event *evt);

  GLMenuBar *pBar;     // Parent menubar

  MENUITEM *items;
  int       nbItem;

  int       selMenu;   // Selected menu
  int       rCode;
  GLMenu   *pMenu;

  BOOL      hasAcc;

};

#endif /* _GLMENUH_ */
