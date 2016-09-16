/*
  File:        GLCombo.h
  Description: ComboBox class (SDL/OpenGL OpenGL application framework)
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
#include "GLList.h"
#include "GLTextField.h"

#ifndef _GLCOMBOH_
#define _GLCOMBOH_

class GLComboPopup;

class GLCombo : public GLComponent {

public:

  // Construction
  GLCombo(int compId);

  // Component methods
  void Clear();
  void SetSize(int nbRow);
  void SetValueAt(int row,const char *value,int userValue=0);
  int  GetUserValueAt(int row);
  void SetSelectedValue(char *value);
  void ScrollTextToEnd();
  void SetSelectedIndex(int idx);
  int  GetSelectedIndex();
  char *GetSelectedValue();
  void SetEditable(BOOL editable);
  char *GetValueAt(int row);
  int  GetNbRow();

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void SetParent(GLContainer *parent);
  void SetFocus(BOOL focus);

  // Expert usage
  GLList *GetList();

private:

  void Drop();

  GLComboPopup *wnd;
  GLList       *list;
  GLTextField  *text;

  int   selectedRow;
  BOOL  m_Editable;
  BOOL  dropped;

};

#endif /* _GLCOMBOH_ */