/*
  File:        GLScrollBar.h
  Description: Scrollbar class (SDL/OpenGL OpenGL application framework)
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

#ifndef _GLSCROOLBARH_
#define _GLSCROOLBARH_

#include "GLComponent.h"

#define SB_VERTICAL   1
#define SB_HORIZONTAL 2

class GLScrollBar : public GLComponent {

public:

  // Construction
  GLScrollBar(int compId);

  // Component methods
  void SetRange(int max,int page,int wheel);
  void SetPosition(int nPos);
  void SetPositionMax();
  int  GetPosition();
  void SetOrientation(int orientation);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  void     SetPos(int nPos);
  void     Measure();

  int      m_Pos;
  int      m_Max;
  int      m_Page;
  int      m_Wheel;
  int      m_Drag;
  int      lastX;
  int      lastY;
  int      orientation;
  int      ws;
  int      ss;
  int      d1,d2;

};

#endif /* _GLSCROOLBARH_ */