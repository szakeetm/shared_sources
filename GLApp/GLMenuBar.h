/*
Program:     MolFlow+
Description: Monte Carlo simulator for ultra-high vacuum
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef _GLMENUBARH_
#define _GLMENUBARH_

#include "GLMenu.h" //MENUITEM

class GLMenuBar : public GLComponent {

public:

  // Construction
  GLMenuBar(int compId);
  ~GLMenuBar();

  // Component methods
  void    Add(char *itemName);
  GLMenu *GetSubMenu(char *itemName);
  void    Close();

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetFocus(bool focus);
  void ProcessAcc(int accId);

  // Expert usage
  void    GoLeft();
  void    GoRight();

private:

  bool  IsInItem(MENUITEM *p,int mx,int my);
  int   GetMenu(int mx,int my);
  void  Drop(int sel);

  MENUITEM items[MAX_MENU_ITEM];
  int nbItem;

  int  selMenu;   // Selected menu
  bool autoDrop; // auto drop flag

};

#endif /* _GLMENUBARH_ */