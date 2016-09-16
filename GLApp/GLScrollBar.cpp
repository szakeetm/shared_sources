/*
  File:        GLButton.cpp
  Description: Button class (SDL/OpenGL OpenGL application framework)
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
#include "GLWindow.h"
#include "GLScrollBar.h"
#include "GLToolkit.h"


// Minimum slider width
#define MIN_SWIDTH 10

// ---------------------------------------------------------------------

GLScrollBar::GLScrollBar(int compId):GLComponent(compId) {
  m_Pos=0;
  m_Max=10;
  m_Page=10;
  m_Wheel=1;
  m_Drag=0;
  orientation = SB_VERTICAL;
  SetBorder(BORDER_NONE);
  SetBackgroundColor(230,222,215);
}

// ---------------------------------------------------------------------

void GLScrollBar::SetRange(int max,int page,int wheel)
{
  if(page>max) {
    m_Max=10;
    m_Page=10;
    m_Wheel=1;
  } else {
    m_Max=max;
    m_Page=page;
    m_Wheel=wheel;
  }
  SATURATE(m_Pos,0,m_Max-m_Page);
  m_Drag=0;
}

// ---------------------------------------------------------------------

void GLScrollBar::SetOrientation(int orientation) {
  this->orientation = orientation;
}

// ---------------------------------------------------------------------

int GLScrollBar::GetPosition() {
  return m_Pos;
}

// ---------------------------------------------------------------------

void GLScrollBar::SetPos(int nPos) {
  SetPosition(nPos);
  parent->ProcessMessage(this,MSG_SCROLL);
}

// ---------------------------------------------------------------------

void GLScrollBar::SetPosition(int nPos) {
   if( (nPos) >= (m_Max-m_Page) ) m_Pos=m_Max-m_Page;
   else                           m_Pos=nPos;
   if(nPos<0) m_Pos=0;
}

// ---------------------------------------------------------------------

void GLScrollBar::SetPositionMax() {
  m_Pos=m_Max-m_Page;
}

// ---------------------------------------------------------------------

void GLScrollBar::Measure() {

  d1 = 0;
  d2 = 0;

  switch(orientation) {

    case SB_VERTICAL:
      // Slider starting position
      ss=(int)( ((double)m_Pos/(double)m_Max) * (double)(height-32) + 0.5 )+16;
      // Slider height
      ws=(int)( (double)(height-32)*(double)m_Page/(double)m_Max + 0.5);
      // Extra offset
      if(ws<MIN_SWIDTH) {
        int m = (MIN_SWIDTH-ws);
        d1 = m/2;  // Up offset
        d2 = m - d1;    // Bottom offset        
      }
      break;

    case SB_HORIZONTAL:
      // Slider starting position
      ss=(int)( ((double)m_Pos/(double)m_Max) * (double)(width-33) + 0.5 )+16;
      // Slider width
      ws=(int)( (double)(width-33)*(double)m_Page/(double)m_Max + 0.5);
      // Extra offset
      if(ws<MIN_SWIDTH) {
        int m = (MIN_SWIDTH-ws);
        d1 = m/2;  // Left offset
        d2 = m - d1;    // Rigth offset
      }
      break;

  }

}

// ---------------------------------------------------------------------

void GLScrollBar::Paint() {

  if(!parent) return;
  Measure();
  GLFont2D *font = GLToolkit::GetDialogFont();

  if( this->IsEnabled() ) font->SetTextColor(0.0f,0.0f,0.0f);
  else                    font->SetTextColor(0.5f,0.5f,0.5f);

  int rB=212,gB=208,bB=200;

  switch(orientation) {

    case SB_VERTICAL:

      // Back
      GLToolkit::DrawHGradientBox(posX,posY,width,height+1);
      GLToolkit::DrawVScroll(posX,posY+15-d1,width-1,height-28+d2,2);

      // Draw up button
      font->DrawText(posX+3,posY+1-d1/2,"\212",FALSE);

      // Draw Slider
      GLToolkit::DrawVScroll(posX,posY+ss-d1,width-1,ws+(d1+d2)+2,m_Drag);

      // Draw down button
      font->DrawText(posX+3,posY+height-14+d2/2,"\211",FALSE);
      break;

    case SB_HORIZONTAL:

      // Back
      GLToolkit::DrawVGradientBox(posX,posY,width,height+1);
      GLToolkit::DrawHScroll(posX+15-d1,posY+1,width-29+d2,height,2);

      // Draw left button
      font->DrawText(posX+6-d1/2,posY,"\214",FALSE);

      // Draw Slider
      GLToolkit::DrawHScroll(posX+ss-d1,posY+1,ws+(d1+d2)+2,height,m_Drag);

      // Draw right button
      font->DrawText(posX+width-10+d2/2,posY,"\213",FALSE);
      break;

  }

}

// ---------------------------------------------------------------------

void GLScrollBar::ManageEvent(SDL_Event *evt) {

  if(!parent) return;
  Measure();
  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt);

  // -------------------------------------------------------------------

  if( evt->type == SDL_MOUSEMOTION ) {
  if( m_Drag ) {

    int diff=(orientation==SB_VERTICAL)?(my-lastY):(mx-lastX);
    int sl = (orientation==SB_VERTICAL)?(height-32):(width-33);

    if( diff ) {
      int d = (int)((double)m_Max*((double)diff/(double)(sl)));
      SetPos(m_Pos+d);
    }

    lastX = mx;
    lastY = my;

  }
  }

  // -------------------------------------------------------------------

  if( evt->type == SDL_MOUSEBUTTONDOWN ) {
  if( evt->button.button == SDL_BUTTON_LEFT ) {

    int mp=(orientation==SB_VERTICAL)?(my):(mx);

    if( mp>ss+ws+(d1+d2) ) {
      // Page increment
       SetPos(m_Pos+m_Page);
    } else if (mp>=ss-d1) {
       //Drag
       lastX = mx;
       lastY = my;
       m_Drag=1;
    } else {
       // Page decrement
       SetPos(m_Pos-m_Page);
    }

  }
  if( evt->button.button == SDL_BUTTON_WHEELUP ) {
    SetPos(m_Pos-m_Wheel);
  }
  if( evt->button.button == SDL_BUTTON_WHEELDOWN ) {
    SetPos(m_Pos+m_Wheel);
  }
  }

  // -------------------------------------------------------------------

  if( evt->type == SDL_MOUSEBUTTONUP ) {
  if( evt->button.button == SDL_BUTTON_LEFT ) {
    m_Drag=0;
  }
  }

  // -------------------------------------------------------------------
  if( evt->type == SDL_KEYDOWN ) {

    int unicode = (evt->key.keysym.unicode & 0x7F);
    if( !unicode ) unicode = evt->key.keysym.sym;

    switch(unicode) {
    case SDLK_PAGEUP:
      SetPos(m_Pos-m_Page);
      break;
    case SDLK_PAGEDOWN:
      SetPos(m_Pos+m_Page);
      break;
    }

  }

}
