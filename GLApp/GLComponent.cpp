/*
  File:        GLComponent.cpp
  Description: Base component class (SDL/OpenGL OpenGL application framework)
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
#include "GLComponent.h"
#include "GLToolkit.h"

// ----------------------------------------------------------

GLComponent::GLComponent(int compId):GLContainer() {

  width=0;
  height=0;
  posX=0;
  posY=0;
  opaque=TRUE;
  border=BORDER_NONE;
  rBack = 212;
  gBack = 208;
  bBack = 200;
  id = compId;
  parent = NULL;
  focus = FALSE;
  enabled = TRUE;
  visible = TRUE;
  focusable = TRUE;
  cursor = CURSOR_DEFAULT;

}

// ----------------------------------------------------------

int GLComponent::GetWidth() {
  return width;
}

int GLComponent::GetHeight() {
  return height;
}

// ----------------------------------------------------------

void GLComponent::SetParent(GLContainer *parent) {

  this->parent =parent;
  SetWindow(parent->GetWindow());

}

// ----------------------------------------------------------

GLContainer *GLComponent::GetParent() {
  return parent;
}

// ----------------------------------------------------------
void GLComponent::SetCursor(int cursor) {
  this->cursor = cursor;
}

// ----------------------------------------------------------
int GLComponent::GetCursor() {
  if( enabled )
    return cursor;
  else
    return CURSOR_DEFAULT;
}

// ----------------------------------------------------------
void GLComponent::SetFocusable(BOOL acceptFocus) {
  focusable = acceptFocus;
}

BOOL GLComponent::IsFocusable() {
  return focusable;
}

void GLComponent::SetPosition(int x,int y) {
  posX = x;
  posY = y;
}

void GLComponent::SetBorder(int border) {
  this->border = border;
}

void GLComponent::SetOpaque(BOOL opaque) {
  this->opaque = opaque;
}

void GLComponent::SetSize(int width,int height) {
  this->width = width;
  this->height = height;
}

void GLComponent::SetBounds(int x,int y,int width,int height) {
  SetSize(width,height);
  SetPosition(x,y);
}

void GLComponent::GetBackgroundColor(int *r,int *g,int *b) {
  *r = rBack;
  *g = gBack;
  *b = bBack;
}

void GLComponent::SetBackgroundColor(int r,int g,int b) {
  rBack = r;
  gBack = g;
  bBack = b;
}

int GLComponent::GetId() {
  return id;
}

void GLComponent::SetFocus(BOOL focus) {
  this->focus = focus;
}

BOOL GLComponent::HasFocus() {
  return focus;
}

void GLComponent::SetEnabled(BOOL enable) {
  enabled = enable;
}

BOOL GLComponent::IsEnabled() {
  return enabled;
}

void GLComponent::SetVisible(BOOL visible) {
  this->visible = visible;
}

BOOL GLComponent::IsVisible() {
  return visible;
}



// ----------------------------------------------------------

void GLComponent::Paint() {
	GLToolkit::CheckGLErrors("GLComponent::Paint()");
  // Paint background
  if(parent && width>0 && height>0 && opaque) {
    switch(border) {
      case BORDER_NONE:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack);
        break;
      case BORDER_BEVEL_IN:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack,TRUE,TRUE);
        break;
      case BORDER_BEVEL_OUT:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack,TRUE,FALSE);
        break;
      case BORDER_ETCHED:
        GLToolkit::DrawBox(posX,posY,width,height,rBack,gBack,bBack,FALSE,FALSE,TRUE);
        break;
    }
  }

}

// ---------------------------------------------------------------

void GLComponent::GetBounds(int *x,int *y,int *w,int *h) {
  *x = posX;
  *y = posY;
  *h = height;
  *w = width;
}
