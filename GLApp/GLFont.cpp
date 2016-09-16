/*
  File:        GLFont.cpp
  Description: Simple 2D bitmap font (SDL/OpenGL OpenGL application framework)
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

#include "GLFont.h"
#include "GLToolkit.h"
#include "GLApp.h"
#include <cimage.h>
#include <malloc.h>
#include <stdio.h>

extern GLApplication *theApp;

// -------------------------------------------

GLFont2D::GLFont2D() {
  strcpy(fileName,"images/font.png");
  cHeight = 15;
  cWidth  = 9;
  isVariable = FALSE;
}

GLFont2D::GLFont2D(char *imgFileName) {
  strcpy(fileName,imgFileName);
  cHeight = 15;
  cWidth  = 9;
  isVariable = FALSE;
}

// -------------------------------------------

int GLFont2D::RestoreDeviceObjects(int scrWidth,int scrHeight) {

  // Load the image
  CImage img;
  if( !img.LoadCImage(fileName) ) {
    char tmp[512];
    sprintf(tmp,"Failed to load \"%s\"",fileName);
    GLToolkit::Log(tmp);
    return 0;
  }

  // Make 32 Bit RGBA buffer
  fWidth  = img.Width();
  fHeight = img.Height();
  BYTE *buff32 = (BYTE *)malloc(fWidth*fHeight*4);
  BYTE *data   = img.GetData();
  for(int y=0;y<fHeight;y++) {
    for(int x=0;x<fWidth;x++) {
      buff32[x*4 + 0 + y*4*fWidth] = data[x*3+2 + y*3*fWidth];
      buff32[x*4 + 1 + y*4*fWidth] = data[x*3+1 + y*3*fWidth];
      buff32[x*4 + 2 + y*4*fWidth] = data[x*3+0 + y*3*fWidth];
      buff32[x*4 + 3 + y*4*fWidth] = data[x*3+1 + y*3*fWidth]; // Green as alpha
    }
  }

  if( isVariable ) {

    // Compute width for each char
    for(int i=0;i<256;i++) {
      cVarWidth[i]=0;
      int xO = ((i % 16) * 16 + 1);
      int yO = ((i / 16) * 16    );

      //scan columns
      BOOL black = TRUE;
      while(black && xO<img.Width()) {
        for(int j=0;j<cHeight && black;j++)
          black = (data[(xO)*3 + (yO+j)*3*fWidth] == 0);
        cVarWidth[i]++;
        xO++;
      }

      BOOL white = TRUE;
      while(white && xO<img.Width()) {
        black = TRUE;
        for(int j=0;j<cHeight && black;j++)
          black = (data[(xO)*3 + (yO+j)*3*fWidth] == 0);
        white = !black;
        if(white) cVarWidth[i]++;
        xO++;
      }

      if(cVarWidth[i]>cWidth) cVarWidth[i]=cWidth;

      //Comprime space char when variable width font
      if(i==32) cVarWidth[i]=cWidth/3;

    }
  }

  glGenTextures(1,&texId);
  glBindTexture(GL_TEXTURE_2D,texId);

  glTexImage2D (
    GL_TEXTURE_2D,      // Type
    0,                  // No Mipmap
    4,                  // Format RGBA
    fWidth,             // Width
    fHeight,            // Height
    0,                  // Border
    GL_RGBA,            // Format RGBA
    GL_UNSIGNED_BYTE,   // 8 Bit/color
    buff32              // Data
  );   

  free(buff32);
  img.Release();

  GLenum glError = glGetError();
  if( glError != GL_NO_ERROR )
  {
    char tmp[512];
    sprintf(tmp,"Failed to create GLFont2D \"%s\"",fileName);
    GLToolkit::Log(tmp);
    GLToolkit::printGlError(glError);
    return 0;
  }

  // Compute othographic matrix (for Transfomed Lit vertex)
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, scrWidth, scrHeight, 0, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );

  
  rC = 1.0f;
  gC = 1.0f;
  bC = 1.0f;

  return 1;

}

// -------------------------------------------

void GLFont2D::ChangeViewport(GLVIEWPORT *g) {
  GLfloat oldProj[16];
  glMatrixMode( GL_PROJECTION );
  glGetFloatv( GL_PROJECTION_MATRIX , oldProj );
  glLoadIdentity();
  glOrtho( g->x, g->x+g->width, g->y+g->height, g->y, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );
  glLoadMatrixf(oldProj);
}

// -------------------------------------------

void GLFont2D::SetVariableWidth(BOOL variable) {
  isVariable = variable;
}

void GLFont2D::SetTextSize(int width,int height) {
  cHeight = height;
  cWidth  = width;
}

int GLFont2D::GetTextWidth(char *text) {
  int lgth = (int)strlen(text);
  int w = 0;

  if( isVariable ) {
    for(int i=0;i<lgth;i++)
      w+=cVarWidth[ (unsigned char)text[i] ];
  } else {
    w = cWidth * lgth;
  }
  return w;
}

int GLFont2D::GetTextHeight() {
  return cHeight;
}

// -------------------------------------------

void GLFont2D::InvalidateDeviceObjects() {

  if(texId) glDeleteTextures(1, &texId);
  texId = 0;

}

// -------------------------------------------

void GLFont2D::SetTextColor(const float &r,const float &g,const float &b) {
  rC = r;
  gC = g;
  bC = b;
}

// -------------------------------------------

void GLFont2D::DrawLargeText(int cx,int cy,char *text,float sizeFactor,BOOL loadMatrix) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;
  int x = cx;
  int y = cy+1;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glColor3f(rC,gC,bC);

  if( loadMatrix ) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  int xcPos=x;
  float cH   = (float)cHeight / (float)fWidth;
  //float factor=1.1f;
	//glScalef(factor,factor,factor);
  glBegin(GL_QUADS);
  for(int i=0;i<lgth;i++ ) {

    unsigned char  c = (unsigned char)text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cWidth;

    } else {
      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2f(xcPos             ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2f(xcPos+cVarWidth[c]*sizeFactor,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2f(xcPos+cVarWidth[c]*sizeFactor,y+cHeight*sizeFactor);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2f(xcPos             ,y+cHeight*sizeFactor);
      xcPos += cVarWidth[c]*sizeFactor;

    }

  }
  
  glEnd();
  //glScalef(1.0f/factor,1.0f/factor,1.0f/factor);
#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}


void GLFont2D::DrawText(const int &cx,const int &cy,char *text,const BOOL &loadMatrix) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;
  int x = cx;
  int y = cy+1;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glColor3f(rC,gC,bC);

  if( loadMatrix ) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  int xcPos=x;
  float cH   = (float)cHeight / (float)fWidth;
  glBegin(GL_QUADS);
  for(int i=0;i<lgth;i++ ) {

    unsigned char  c = (unsigned char)text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cWidth;

    } else {

      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos             ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cVarWidth[c],y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cVarWidth[c],y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos             ,y+cHeight);
      xcPos += cVarWidth[c];

    }

  }
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}

// -------------------------------------------

void GLFont2D::DrawTextFast(int cx,int cy,char *text) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;
  int x = cx;
  int y = cy+1;

  glBindTexture(GL_TEXTURE_2D,texId);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
  glColor3f(rC,gC,bC);

  int xcPos=x;
  float cH   = (float)cHeight / (float)fWidth;
  glBegin(GL_QUADS);
  for(int i=0;i<lgth;i++ ) {

    unsigned char  c = (unsigned char)text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cWidth;

    } else {

      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos             ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cVarWidth[c],y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cVarWidth[c],y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos             ,y+cHeight);
      xcPos += cVarWidth[c];

    }

  }
  glEnd();

#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::DrawTextV(int x,int y,char *text,BOOL loadMatrix) {

  int lgth = (int)strlen(text);
  if( lgth==0 ) return;

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D,texId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glColor3f(rC,gC,bC);
  if( loadMatrix ) {
    glMatrixMode( GL_PROJECTION );
    glLoadMatrixf(pMatrix);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
  }

  int ycPos=y;
  for(int i=0;i<lgth;i++ ) {

    char  c = text[i];
    float xPos = (float)((c % 16) * 16 + 1)/ (float)fWidth;
    float yPos = (float)((c / 16) * 16 )/ (float)fHeight;

    if(!isVariable) {

      float cW   = (float)cWidth / (float)fWidth;
      float cH   = (float)cHeight / (float)fWidth;
      glBegin(GL_QUADS);
      glTexCoord2f(xPos   ,yPos   );glVertex2i(x        ,ycPos       );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(x        ,ycPos-cWidth);
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(x+cHeight,ycPos-cWidth);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(x+cHeight,ycPos       );
      glEnd();
      ycPos += cWidth;

    } else {

      float cW   = (float)cVarWidth[c] / (float)fWidth;
      float cH   = (float)cHeight / (float)fWidth;
      glBegin(GL_QUADS);
      glTexCoord2f(xPos   ,yPos   );glVertex2i(x        ,ycPos             );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(x        ,ycPos-cVarWidth[c]);
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(x+cHeight,ycPos-cVarWidth[c]);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(x+cHeight,ycPos             );
      glEnd();
      ycPos -= cVarWidth[c];

    }

  }

#ifdef _DEBUG
  theApp->nbPoly+=lgth;
#endif

}
