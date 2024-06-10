#pragma once
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_opengl.h>
#include "GLTypes.h"

class GLFont2DTTF {

public:

  // Default constructor
  GLFont2DTTF();
  GLFont2DTTF(const std::string& _fileName);
  
  // Initialise the font
  // return 1 when success, 0 otherwise
  int RestoreDeviceObjects(int srcWidth,int scrHeight);
  
  // Draw a 2D text (in viewport coordinates)
  void GLDrawText(const int x,const int y,const char *text,const bool loadMatrix=true);
  void GLDrawLargeText(int x,int y,const char *text,float sizeFactor,bool loadMatrix=true);
  void GLDrawTextFast(int cx,int cy,const char *text);
  void GLDrawTextV(int x,int y,char *text,bool loadMatrix=true);

  // Release any allocated resource
  void InvalidateDeviceObjects();

  // Set text color
  void SetTextColor(const float r,const float g,const float b);

  // Set default character size (Default 9,15)
  void SetTextSize(int width,int height);

  // Set variable font width (Must be called before RestoreDeviceObject)
  void SetVariableWidth(bool variable);

  // Get string size
  int GetTextWidth(const char *text);
  int GetTextHeight();

  // Adapt orthographic projection on viewport change
  void ChangeViewport(GLVIEWPORT *g);

private:

  std::string fileName;
  int fontWidth,fontHeight,maxCharWidth=9,maxCharHeight=15;
  GLCOLOR color(1.0f,1.0f,1.0f,1.0f);
  GLfloat pMatrix[16];
  bool    isVariable=false;
  int fontSize=24;
  GLuint  textureId; //opengl texture
};
