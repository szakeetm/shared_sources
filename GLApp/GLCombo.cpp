/*
  File:        GLCombo.cpp
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
#include "GLWindow.h"
#include "GLCombo.h"
#include "GLToolkit.h"
#include "GLWindowManager.h"

// Popup -----------------------------------------------------------

class GLComboPopup : public GLWindow {

public:

  GLComboPopup(GLCombo *parent):GLWindow() {
    this->parent = parent;
    SetAnimatedFocus(FALSE);
    rCode=0;
  }

  void ProcessMessage(GLComponent *src,int message) {
    if( message == MSG_LIST ) {
      rCode = 1; // Select
      GLWindow::ProcessMessage(NULL,MSG_CLOSE);
    }
  }

  void ManageEvent(SDL_Event *evt) {
    if( evt->type == SDL_MOUSEBUTTONDOWN ) {
      int mx = GetX(NULL,evt);
      int my = GetY(NULL,evt);
      if(mx>=0 && mx<=width && my>=0 && my<=height) {
        GLWindow::ManageEvent(evt);
      } else {
        // Click outside => cancel
        rCode = 2;
        GLWindow::ProcessMessage(NULL,MSG_CLOSE);
      }
    } else {
      GLWindow::ManageEvent(evt);
    }
  }

  int  rCode;
  GLCombo *parent;

};

// -----------------------------------------------------------------

GLCombo::GLCombo(int compId):GLComponent(compId) {

  wnd = new GLComboPopup(this);
  list = new GLList(compId);
  list->SetMotionSelection(TRUE);
  wnd->Add(list);
  text = new GLTextField(compId,"");
  selectedRow = -1;
  m_Editable = FALSE;
  text->SetEditable_NoBG(m_Editable);
  dropped = FALSE;
  SetBorder(BORDER_BEVEL_IN);
  text->SetBorder(BORDER_NONE);
  Add(text);

}

// -----------------------------------------------------------------

void GLCombo::Clear() {
  list->Clear();
}

// -----------------------------------------------------------------

GLList *GLCombo::GetList() {
  return list;
}

// -----------------------------------------------------------------

void GLCombo::SetSize(int nbRow) {
  if( nbRow==0 )
    list->Clear();
  else
    list->SetSize(1,nbRow);
}

// -----------------------------------------------------------------

void GLCombo::SetValueAt(int row,const char *value,int userValue) {
  list->SetValueAt(0,row,value,userValue);
}

// -----------------------------------------------------------------

int GLCombo::GetUserValueAt(int row) {
  return list->GetUserValueAt(0,row);
}

// -----------------------------------------------------------------

char *GLCombo::GetValueAt(int row) {
  return list->GetValueAt(0,row);
}

// -----------------------------------------------------------------

int GLCombo::GetNbRow() {
  return list->GetNbRow();
}

// -----------------------------------------------------------------

void GLCombo::SetSelectedValue(char *value) {
  text->SetText(value);
  selectedRow = -1;
}

// -----------------------------------------------------------------

char *GLCombo::GetSelectedValue() {
  return text->GetText();
}

// -----------------------------------------------------------------

void GLCombo::ScrollTextToEnd() {
  char *value = text->GetText();
  int l = (value?(int)strlen(value):0);
  text->SetCursorPos(l);
  text->ScrollToVisible();
}

// -----------------------------------------------------------------

void GLCombo::SetSelectedIndex(int idx) {
	if (idx == -1) {
		text->SetText("");
	}
	else {
		text->SetText(list->GetValueAt(0, idx));
	}
  selectedRow = idx;
}

// -----------------------------------------------------------------

int GLCombo::GetSelectedIndex() {
  return selectedRow;
}

// -----------------------------------------------------------------

void GLCombo::SetEditable(BOOL editable) {
  m_Editable = editable;
  SetEnabled(editable);
  text->SetEditable(m_Editable);
}

// -----------------------------------------------------------------

void GLCombo::Paint() {

  if(!parent) return;
  int backGroundColor=enabled?240:210;
  GLToolkit::DrawTextBack(posX,posY,width,height+2,backGroundColor,backGroundColor,backGroundColor);
  GLComponent::PaintComponents();
  GLFont2D *font = GLToolkit::GetDialogFont();

  // Draw down button
  if( this->IsEnabled() ) font->SetTextColor(0.0f,0.0f,0.0f);
  else                    font->SetTextColor(0.5f,0.5f,0.5f);
  GLToolkit::DrawSmallButton(posX+width-16,posY+1,dropped);
  if(dropped) {
    //GLToolkit::DrawBox(posX+width-16,posY+1,15,height-2,rBack,gBack,bBack,TRUE,TRUE);
    font->DrawText(posX+width-11,posY+4,"\211",FALSE);
  } else {
    //GLToolkit::DrawBox(posX+width-16,posY+1,15,height-2,rBack,gBack,bBack,TRUE);
    font->DrawText(posX+width-12,posY+3,"\211",FALSE);
  }

}

// -----------------------------------------------------------------

void GLCombo::Drop() {

  dropped = TRUE;
  Paint();

  // Old clipping
  GLint   old_v[4];
  GLfloat old_m[16];
  glGetFloatv( GL_PROJECTION_MATRIX , old_m );
  glGetIntegerv( GL_VIEWPORT , old_v );

  int x = GetWindow()->GetScreenX(this);
  int y = GetWindow()->GetScreenY(this);
  int h = list->GetNbRow()*15+4;
  BOOL needScroll = FALSE;
  if( h>116 ) { h=116;needScroll=TRUE; }

  int ws[] = { width-4 };
  list->SetColumnWidths(ws);
  list->SetVScrollVisible(needScroll);
  list->SetHScrollVisible(FALSE);
  list->SetBounds(1,1,width-2,h-2);
  list->SetFocus(TRUE);

  // Place drop list
  int wScr,hScr;
  GLToolkit::GetScreenSize(&wScr,&hScr);
  if( y+height+h<hScr ) {
    wnd->SetBounds(x,y+height+1,width,h);
  } else {
    wnd->SetBounds(x,y-h-1,width,h);    
  }

  wnd->RestoreDeviceObjects();
  wnd->DoModal();

  if(wnd->rCode==1) {
    int sRow = list->GetSelectedRow();
    if(sRow>=0) {
      SetSelectedValue(list->GetValueAt(0,sRow));
      selectedRow = sRow;
    }
    parent->ProcessMessage(this,MSG_COMBO);
  }

  wnd->InvalidateDeviceObjects();
  GLWindowManager::FullRepaint();

  glMatrixMode( GL_PROJECTION );
  glLoadMatrixf(old_m);
  glViewport(old_v[0],old_v[1],old_v[2],old_v[3]);

  dropped = FALSE;

}

// -----------------------------------------------------------------

void GLCombo::SetFocus(BOOL focus) {
  if(!focus) {
    dropped=FALSE;
  }
  text->SetFocus(focus);
  GLComponent::SetFocus(focus);
}

// -----------------------------------------------------------------

void GLCombo::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int mx = GetWindow()->GetX(this,evt);
  int my = GetWindow()->GetY(this,evt);

  if( mx>=width-16 || !m_Editable ) {
    if( evt->type == SDL_MOUSEBUTTONDOWN ) {
      int nbRow = list->GetNbRow();
      if( evt->button.button==SDL_BUTTON_WHEELUP ) {
        if( nbRow>0 && selectedRow>0 ) {
          SetSelectedIndex(selectedRow-1);
          if(parent) parent->ProcessMessage(this,MSG_COMBO);
        }
      } else if( evt->button.button==SDL_BUTTON_WHEELDOWN ) {
        if( nbRow>0 && selectedRow<nbRow-1 ) {
          SetSelectedIndex(selectedRow+1);
          if(parent) parent->ProcessMessage(this,MSG_COMBO);
        }
      } else {
        Drop();
      }
    }
    return;
  }

  text->ManageEvent(evt);

}

// -----------------------------------------------------------------

void GLCombo::SetBounds(int x,int y,int width,int height) {

  // Place text field
  text->SetBounds(x+2,y+2,width-18,height-2);
  GLComponent::SetBounds(x,y,width,height);

}

// -----------------------------------------------------------------

void GLCombo::SetParent(GLContainer *parent) {
  text->SetParent(parent);
  GLComponent::SetParent(parent);
}
