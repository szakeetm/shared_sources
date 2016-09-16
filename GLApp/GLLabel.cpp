/*
  File:        GLLabel.cpp
  Description: Label class (SDL/OpenGL OpenGL application framework)
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
#include "GLLabel.h"
#include "GLToolkit.h"

#include <malloc.h>

// ---------------------------------------------------------------------

GLLabel::GLLabel(const char *text):GLComponent(0) {

  mText = NULL;
  nbLine=0;
  font = GLToolkit::GetDialogFont();
  SetTextColor(0,0,0);
  SetText(text);

}

// ---------------------------------------------------------------------

void GLLabel::Clear() {

  SAFE_FREE(mText);
  txtWidth = 0;
  txtHeight = 0;
  mText=NULL;
  nbLine=0;

}

// --------------------------------------------------------------------
char* GLLabel::GetText() {
	return mText;
}

// ---------------------------------------------------------------------

void GLLabel::SetText(const char *text) {


  if( !text ) {
    Clear();
    return;
  } 
  
  if(text[0]==0) {
    Clear();
    return;
  }

  if( mText && strcmp(mText,text)==0 ) {
    // nothing to do
    return;
  }

  Clear();

  mText=_strdup(text);

  // Split in multiline message
  int w;
  char *p;
  char *m = mText;
  while( (p = strchr(m,'\n'))!=NULL ) {
    *p = 0;
    lines[nbLine++] = m;
    w = font->GetTextWidth(m);
    txtWidth = MAX(txtWidth,w);
    m = p+1;
  }

  // Last line
  lines[nbLine++] = m;
  w = font->GetTextWidth(m);
  txtWidth = MAX(txtWidth,w);
  txtHeight = 14 * nbLine;

}

void GLLabel::RestoreDeviceObjects() {
  font = GLToolkit::GetDialogFont();
}

// ---------------------------------------------------------------------

GLLabel::~GLLabel() {
  Clear();
}

// ---------------------------------------------------------------------

void GLLabel::SetTextColor(int r,int g,int b) {
  rText = r/255.0f;
  gText = g/255.0f;
  bText = b/255.0f;
}

// ---------------------------------------------------------------------

void GLLabel::GetTextBounds(int *w,int *h) {
 *w = txtWidth;
 *h = txtHeight;
}

// ---------------------------------------------------------------------

void GLLabel::Paint() {

  if(!parent) return;

  GLComponent::Paint();

  //Message
  font->SetTextColor(rText,gText,bText);
  for(int i=0;i<nbLine;i++)
    font->DrawText(posX,posY+14*i+2,lines[i],FALSE);
  GLToolkit::CheckGLErrors("GLLabel::Paint()");
}

GLOverlayLabel::GLOverlayLabel(char *text):GLLabel(text) {

  sizeFactor=3.0f;
  paintBg=FALSE;
}

void GLOverlayLabel::SetBackgroundColor(float r,float g,float b) {
	rBack =r;
	gBack=g;
	bBack=b;
}

void GLOverlayLabel::Paint() {
	if(!parent) return;

	if (paintBg) GLComponent::Paint();
	//GLComponent::Paint();

	//Message
	font->SetTextColor(rText,gText,bText);
	//font->SetTextSize((int)(size*0.6),size);
	for(int i=0;i<nbLine;i++)
		font->DrawLargeText(posX,posY+14*i+2,lines[i],sizeFactor,FALSE);
	//font->SetTextSize(9,15);
	GLToolkit::CheckGLErrors("GLOverlayLabel::Paint()");
}

void GLOverlayLabel::SetTextSize(float s) {
	sizeFactor=s;
}