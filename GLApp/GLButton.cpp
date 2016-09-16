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
#include "GLButton.h"
#include "GLToolkit.h"

// ---------------------------------------------------------------------

GLButton::GLButton(int compId,char *text):GLComponent(compId) {
  SetText(text);
  state=0;
  toggle=FALSE;
  toggleState=FALSE;
  icon = NULL;
  iconD = NULL;
  strcpy(iconName,"");
  strcpy(iconNameDisa,"");
  font = GLToolkit::GetDialogFont();
  r = g = b = 0.0f; //default color: black
}

void GLButton::SetFontColor(int red, int green, int blue){
	SetFontColor((float)red / 255.0f, (float)green / 255.0f, (float)blue / 255.0f);
}

void GLButton::SetFontColor(float red, float green, float blue){
	r=red;
	g = green;
	b = blue;
}

// ----------------------------------------------------------

void GLButton::InvalidateDeviceObjects() {
  if(icon) {
    icon->InvalidateDeviceObjects();
    SAFE_DELETE(icon);
  }
  if(iconD) {
    iconD->InvalidateDeviceObjects();
    SAFE_DELETE(iconD);
  }
}

// ----------------------------------------------------------

void GLButton::RestoreDeviceObjects() {

  if(strlen(iconName)>0) {
    icon = new Sprite2D();
    icon->RestoreDeviceObjects(iconName,"none",16,16);
    icon->SetSpriteMapping(0.0f,0.0f,1.0f,1.0f);
    icon->UpdateSprite(posX+1+state,posY+1+state,posX+17+state,posY+17+state);
  }

  if(strlen(iconNameDisa)>0) {
    iconD = new Sprite2D();
    iconD->RestoreDeviceObjects(iconNameDisa,"none",16,16);
    iconD->SetSpriteMapping(0.0f,0.0f,1.0f,1.0f);
    iconD->UpdateSprite(posX+1+state,posY+1+state,posX+17+state,posY+17+state);
  }
  font = GLToolkit::GetDialogFont();

}

void GLButton::SetText(char *text) {
  if(text) 
    strcpy(this->text,text);
  else
    strcpy(this->text,"");
}

// ---------------------------------------------------------------------

void GLButton::SetToggle(BOOL toggle) {
  this->toggle = toggle;
  if( !toggle ) toggleState = FALSE;
}

// ---------------------------------------------------------------------

BOOL GLButton::GetState() {
  return toggleState;
}

// ---------------------------------------------------------------------

void GLButton::SetState(BOOL checked) {
  toggleState = checked;
  toggle = TRUE;
}

// ---------------------------------------------------------------------

void GLButton::Paint() {

  if(!parent) return;

  if(state || toggleState) {
    SetBorder(BORDER_BEVEL_IN);
  } else {
    // Released
    SetBorder(BORDER_BEVEL_OUT);
  }

  if( icon ) {

    GLComponent::Paint();
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if( !enabled && iconD ) 
      iconD->Render(FALSE);
    else
      icon->Render(FALSE);

  } else {

    GLToolkit::DrawButtonBack(posX,posY-1,width,height+2,state||toggleState);

    int w = font->GetTextWidth(text);
    int sw = posX + (width-w)/2 - 1;

    if( !enabled ) {

      font->SetTextColor(1.0f,1.0f,1.0f);
      font->DrawText(sw+1,posY+4,text,FALSE);
      font->SetTextColor(0.4f,0.4f,0.4f);
      font->DrawText(sw,posY+3,text,FALSE);

    } else {

      font->SetTextColor(r,g,b);
      if(state || toggleState)
        font->DrawText(sw+1,posY+4,text,FALSE);
      else
        font->DrawText(sw,posY+3,text,FALSE);

    }

  }

}

// ---------------------------------------------------------------------

void GLButton::SetBounds(int x,int y,int width,int height) {
  GLComponent::SetBounds(x,y,width,height);
  if( icon ) icon->UpdateSprite(posX+1+state,posY+1+state,posX+17+state,posY+17+state);
  if( iconD ) iconD->UpdateSprite(posX+1+state,posY+1+state,posX+17+state,posY+17+state);
}

// ---------------------------------------------------------------------

void GLButton::SetIcon(char *fileName) {
  strncpy(iconName,fileName,256);
}

void GLButton::SetDisabledIcon(char *fileName) {
  strncpy(iconNameDisa,fileName,256);
}

// ---------------------------------------------------------------------

void GLButton::ManageEvent(SDL_Event *evt) {

  if(!parent) return;

  int x,y,w,h;

  GetWindow()->GetBounds(&x,&y,&w,&h);
  int px = GetWindow()->GetX(this,evt);
  int py = GetWindow()->GetY(this,evt);

  if( evt->type == SDL_MOUSEBUTTONDOWN ) {
    if( evt->button.button == SDL_BUTTON_LEFT ) {
      state = 1;
    }
  }

  if( evt->type == SDL_MOUSEBUTTONUP ) {
    if( toggle ) {
      if( evt->button.button == SDL_BUTTON_LEFT && state ) {
        if( px>=0 && px<width && py>=0 && py<height ) {
          parent->ProcessMessage(this,MSG_BUTTON);
          toggleState = !toggleState;
        }
      }
    } else {
      if( evt->button.button == SDL_BUTTON_LEFT ) {
        if( px>=0 && px<width && py>=0 && py<height && state )
          parent->ProcessMessage(this,MSG_BUTTON);
      }
    }
    state = 0;
  }

  if(icon) icon->UpdateSprite(posX+1+state,posY+1+state,posX+17+state,posY+17+state);


}
