/*
  File:        GLTextField.h
  Description: Text fiedl class (SDL/OpenGL OpenGL application framework)
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
#include <string>

#ifndef _GLTEXTFIELDH_
#define _GLTEXTFIELDH_

#define MAX_TEXT_SIZE 1024

class GLTextField : public GLComponent {

public:

  // Construction
  GLTextField(int compId,char *text);

  // Component methods
  void SetText(const char *text);
  void SetText(std::string string);
  void SetText(const double &val);
  void SetText(const int &val);
  char *GetText();
  void ScrollToVisible();
  void SetCursorPos(int pos);
  int  GetCursorPos();
  int  GetTextLength();
  void SetEditable(BOOL editable);
  BOOL IsEditable();
  void SetEditable_NoBG(BOOL editable);
  void SetTextColor(float r, float g, float b);
  void Clear();
  // ------------------------------------------------------


  BOOL GetNumber(double *num);
  BOOL GetNumberInt(int *num);
  void SelectAll();
  BOOL IsCaptured();

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetFocus(BOOL focus);

private:

  void   CopyClipboardText();
  void   PasteClipboardText();
  void   UpdateXpos();
  void   InsertString(char *lpszString);
  void   DeleteString(int count);
  void   MoveCursor(int newPos);
  void   RemoveSel();
  void   MoveSel(int newPos);
  void   DeleteSel();
  void   UpdateText(const char *text);
  void	 ProcessEnter();
  int    GetCursorLocation(int px);

  char    m_Text[MAX_TEXT_SIZE];
  int      m_Start;
  int      m_Stop;
  int      m_Length;
  int      m_CursorPos;
  short    m_XPos[MAX_TEXT_SIZE];
  int      m_CursorState;
  int      m_Captured;
  int      m_LastPos;
  int     m_Zero;
  BOOL    m_Editable;
  float   rText, gText, bText;

};

#endif /* _GLTEXTFIELDH_ */