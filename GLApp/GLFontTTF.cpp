
#include "GLFontTTF.h"
#include "GLToolkit.h"
#include "GLApp.h"

#include <stdio.h>
#include <cstring> //strcpy, etc.

extern GLApplication *theApp;

GLFont2DTTF::GLFont2DTTF() : fileName{}, pMatrix{}, cVarWidth{}{
  fileName="fonts/FreeMono.ttf";
}

GLFont2D::GLFont2D(const std::string _fileName, int _fontSize) : fileName{_fileName}, fontSize(_fontSize), pMatrix{} {
}

int GLFont2D::RestoreDeviceObjects(int scrWidth,int scrHeight) {
    // Create SDL renderer
    SDL_Renderer* ren = SDL_CreateRenderer(theApp->mainScreen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 0;
    }

  try{
    TTF_Font* font = TTF_OpenFont(fileName.c_str(), fontSize);
    if (font == nullptr) {
          std::string errorMsg = fmt::format("Failed to load {}:\n{}", fileName, TTF_GetError());
          GLToolkit::Log(errorMsg.c_str());
          return 0;
    }


	}
	catch (...) {
		char tmp[600];
		sprintf(tmp, "Failed to load \"%s\"", fileName);
		GLToolkit::Log(tmp);
		return 0;
	}



  GLenum glError = glGetError();
  if( glError != GL_NO_ERROR )
  {
    char tmp[600];
    sprintf(tmp,"Failed to create GLFont2D \"%s\"",fileName.c_str());
    GLToolkit::Log(tmp);
    GLToolkit::printGlError(glError);
    return 0;
  }

  // Compute othographic matrix (for Transformed Lit vertex)
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0, scrWidth, scrHeight, 0, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );

  return 1;

}

void GLFont2D::ChangeViewport(GLVIEWPORT *g) {
  GLfloat oldProj[16];
  glMatrixMode( GL_PROJECTION );
  glGetFloatv( GL_PROJECTION_MATRIX , oldProj );
  glLoadIdentity();
  glOrtho( g->x, g->x+g->width, g->y+g->height, g->y, -1, 1 );
  glGetFloatv( GL_PROJECTION_MATRIX , pMatrix );
  glLoadMatrixf(oldProj);
}

void GLFont2D::SetVariableWidth(bool variable) {
  isVariable = variable;
}

void GLFont2D::SetTextSize(int width,int height) {
  maxCharHeight = height;
  maxCharHeight  = width;
}

int GLFont2D::GetTextWidth(const char *text) {
    int lgth = text ? (int)strlen(text) : 0;
  int w = 0;

  if( isVariable ) {
    for(int i=0;i<lgth;i++)
      w+=cVarWidth[ (unsigned char)text[i] ];
  } else {
    w = cMaxWidth * lgth;
  }
  return w;
}

int GLFont2D::GetTextHeight() {
  return fontHeight;
}

void GLFont2D::InvalidateDeviceObjects() {

  if(texId) glDeleteTextures(1, &texId);
  texId = 0;

}

void GLFont2D::SetTextColor(const float r,const float g,const float b) {
  color.r=r;
  color.g=g;
  color.b=b;
}

void GLFont2D::GLDrawLargeText(int cx,int cy,const char *text,float sizeFactor,bool loadMatrix) {

    int lgth = text ? (int)strlen(text) : 0;
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

      float cW   = (float)cMaxWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cMaxWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cMaxWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cMaxWidth;

    } else {
      float cW   = (float)cVarWidth[c] / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2f((float)xcPos             , (float)y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2f((float)xcPos+cVarWidth[c]*sizeFactor, (float)y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2f((float)xcPos+cVarWidth[c]*sizeFactor, (float)y+cHeight*sizeFactor);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2f((float)xcPos             , (float)y+cHeight*sizeFactor);
      xcPos += (int)(cVarWidth[c]*sizeFactor);

    }

  }
  
  glEnd();
  //glScalef(1.0f/factor,1.0f/factor,1.0f/factor);
#if defined(DEBUG)
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::GLDrawText(const int cx,const int cy,const char *text,const bool loadMatrix) {

    // Create a surface with the text
    SDL_Color color = {color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderText_Solid(font, "Hello, SDL_ttf!", color);
    if (surface == nullptr) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
    }

        // Create a texture from the surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
    }



    int lgth = text ? (int)strlen(text) : 0;
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

      float cW   = (float)cMaxWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cMaxWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cMaxWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cMaxWidth;

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

#if defined(DEBUG)
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::GLDrawTextFast(int cx,int cy,const char *text) {

  int lgth = text ? (int)strlen(text) : 0;
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

      float cW   = (float)cMaxWidth / (float)fWidth;
      glTexCoord2f(xPos   ,yPos   );glVertex2i(xcPos       ,y   );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(xcPos+cMaxWidth,y   );
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(xcPos+cMaxWidth,y+cHeight);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(xcPos       ,y+cHeight);
      xcPos += cMaxWidth;

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

#if defined(DEBUG)
  theApp->nbPoly+=lgth;
#endif

}

void GLFont2D::GLDrawTextV(int x,int y,char *text,bool loadMatrix) {

    int lgth = text ? (int)strlen(text) : 0;
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

      float cW   = (float)cMaxWidth / (float)fWidth;
      float cH   = (float)cHeight / (float)fWidth;
      glBegin(GL_QUADS);
      glTexCoord2f(xPos   ,yPos   );glVertex2i(x        ,ycPos       );
      glTexCoord2f(xPos+cW,yPos   );glVertex2i(x        ,ycPos-cMaxWidth);
      glTexCoord2f(xPos+cW,yPos+cH);glVertex2i(x+cHeight,ycPos-cMaxWidth);
      glTexCoord2f(xPos   ,yPos+cH);glVertex2i(x+cHeight,ycPos       );
      glEnd();
      ycPos += cMaxWidth;

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

#if defined(DEBUG)
  theApp->nbPoly+=lgth;
#endif

}
