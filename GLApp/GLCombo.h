// Copyright (c) 2011 rubicon IT GmbH
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
  void SetSize(int nbRow);
  void SetValueAt(int row, const std::string& value, int userValue = 0);
  int  GetUserValueAt(int row);
  void SetSelectedValue(const char *value);
  void ScrollTextToEnd();
  void SetSelectedIndex(int idx);
  int  GetSelectedIndex();
  std::string GetSelectedValue();
  void SetEditable(bool editable);
  std::string GetValueAt(int row);
  int  GetNbRow();

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