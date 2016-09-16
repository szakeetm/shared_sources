/*
  File:        GLTitledPanel.cpp
  Description: Titled panel class (SDL/OpenGL OpenGL application framework)
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
#include "GLTitledPanel.h"
#include "GLToolkit.h"

// ---------------------------------------------------------------------

GLTitledPanel::GLTitledPanel(char *title):GLComponent(0) {

  isBold = TRUE;
  SetTitle(title);
  opaque = FALSE;
  SetTextColor(140,140,140);
  closeAble = FALSE;
  closeState = 0;
  closed = FALSE;
  wOrg = 100;
  hOrg = 30;

}

// ---------------------------------------------------------------------

void GLTitledPanel::SetTitle(char *title) {

  if(title) {
    strcpy(this->title,title);
  }  else {
    strcpy(this->title,"");
  }
  if( isBold ) txtWidth = GLToolkit::GetDialogFontBold()->GetTextWidth(this->title);
  else         txtWidth = GLToolkit::GetDialogFont()->GetTextWidth(this->title);

}

// ---------------------------------------------------------------------

void GLTitledPanel::SetBounds(int x,int y,int width,int height) {

  if(!closed) {
    GLComponent::SetBounds(x,y,width,height);
    wOrg = width;
    hOrg = height;
  } else {
    GLComponent::SetBounds(x,y,width,20);
  }

}

// ---------------------------------------------------------------------

void GLTitledPanel::SetBold(BOOL b) {
  isBold = b;
  if( isBold ) txtWidth = GLToolkit::GetDialogFontBold()->GetTextWidth(title);
  else         txtWidth = GLToolkit::GetDialogFont()->GetTextWidth(title);
}

// ---------------------------------------------------------------------

void GLTitledPanel::SetClosable(BOOL c) {
  closeAble = c;
}

// ---------------------------------------------------------------------

void GLTitledPanel::Close() {
  closed = TRUE;
}

// ---------------------------------------------------------------------

void GLTitledPanel::Open() {
  closed = FALSE;
}

// ---------------------------------------------------------------------

BOOL GLTitledPanel::IsClosed() {
  return closed;
}

// ---------------------------------------------------------------------

void GLTitledPanel::SetTextColor(int r,int g,int b) {
  rText = r/255.0f;
  gText = g/255.0f;
  bText = b/255.0f;
}

// ---------------------------------------------------------------------

void GLTitledPanel::ManageEvent(SDL_Event *evt) {

  GLContainer::ManageEvent(evt);
  GLContainer::RelayEvent(evt);

  int mX = GetWindow()->GetX(this,evt);
  int mY = GetWindow()->GetY(this,evt);

  if( evt->type==SDL_MOUSEBUTTONUP ) {
    closeState = 0;
  }

  if( mX>=7 && mX<=19+txtWidth && mY>=1 && mY<=13 && !evtProcessed ) {
    if( evt->type==SDL_MOUSEBUTTONDOWN && evt->button.button==SDL_BUTTON_LEFT ) {
      closeState = 1;
    }
    if( evt->type==SDL_MOUSEBUTTONUP && evt->button.button==SDL_BUTTON_LEFT ) {
      closed = !closed;
      parent->ProcessMessage(this,MSG_PANELR);
    }
  }


}

// ---------------------------------------------------------------------

void GLTitledPanel::ProcessMessage(GLComponent *src,int message) {
  // Relay to parent
  parent->ProcessMessage(src,message);
}


// ---------------------------------------------------------------------

void GLTitledPanel::SetCompBounds(GLComponent *src,int x,int y,int w,int h) {
  src->SetBounds(x+posX,y+posY,w,h);
}

// ---------------------------------------------------------------------

void GLTitledPanel::Paint() {

  if(!parent) return;

  if( opaque ) {
    GLToolkit::DrawBox(posX,posY+6,width,height-6,rBack,gBack,bBack,FALSE,FALSE,TRUE);
  } else {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glLineWidth(1.0f);
    GLToolkit::DrawBorder(posX,posY+6,width,height-6,FALSE,FALSE,TRUE);
  }

  if( strlen(title) ) {
    int cw = (closeAble)?12:0;
    if( isBold ) {
      GLToolkit::DrawBox(posX+5+cw,posY,txtWidth+10,15,rBack,gBack,bBack);
      GLToolkit::GetDialogFontBold()->SetTextColor(rText,gText,bText);
      GLToolkit::GetDialogFontBold()->DrawText(posX+10+cw,posY,title,FALSE);
    } else {
      GLToolkit::DrawBox(posX+5+cw,posY,txtWidth+10,15,rBack,gBack,bBack);
      GLToolkit::GetDialogFont()->SetTextColor(rText,gText,bText);
      GLToolkit::GetDialogFont()->DrawText(posX+10+cw,posY,title,FALSE);
    }
    if( closeAble ) {
      GLToolkit::DrawTinyButton(posX+7,posY+1,closeState);
      GLToolkit::GetDialogFont()->SetTextColor(0,0,0);
      if( closed ) {
        GLToolkit::GetDialogFont()->DrawText(posX+10,posY-1,"+",FALSE);
      } else {
        GLToolkit::GetDialogFont()->DrawText(posX+10,posY-1,"-",FALSE);
      }
    }
  }

  if(!closed) GLComponent::PaintComponents();

}
