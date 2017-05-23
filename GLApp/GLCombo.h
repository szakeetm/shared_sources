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

#ifndef _GLCOMBOH_
#define _GLCOMBOH_

#include "GLComponent.h"
class GLList;
class GLTextField;
class GLComboPopup;

class GLCombo : public GLComponent {

public:

  // Construction
  GLCombo(int compId);

  // Component methods
  void Clear();
  void SetSize(size_t nbRow);
  void SetValueAt(size_t row,const char *value,int userValue=0);
  int  GetUserValueAt(size_t row);
  void SetSelectedValue(char *value);
  void ScrollTextToEnd();
  void SetSelectedIndex(int idx);
  int  GetSelectedIndex();
  char *GetSelectedValue();
  void SetEditable(bool editable);
  char *GetValueAt(size_t row);
  size_t  GetNbRow();

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void SetParent(GLContainer *parent);
  void SetFocus(bool focus);

  // Expert usage
  GLList *GetList();

private:

  void Drop();

  GLComboPopup *wnd;
  GLList       *list;
  GLTextField  *text;

  int   selectedRow;
  bool  m_Editable;
  bool  dropped;

};

#endif /* _GLCOMBOH_ */