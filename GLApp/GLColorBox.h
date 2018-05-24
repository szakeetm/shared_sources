// Copyright (c) 2011 rubicon IT GmbH

//#include <SDL_opengl.h>

#ifndef _GLCOLORBOXH_
#define _GLCOLORBOXH_

class GLColorBox : private GLWindow {

public:
  // Display a modal dialog and return 1 on ok, 0 on cancel. r,g and b 
  // contains the choosen color
  static int Display(char *title,int *r,int *g,int *b);

  int  rCode;
  int  curR;
  int  curG;
  int  curB;

  // Implementation
  void InvalidateDeviceObjects();
  void RestoreDeviceObjects();

private:

  GLTitledPanel *swPanel;
  GLTitledPanel *hsvPanel;
  GLTextField   *rText;
  GLTextField   *gText;
  GLTextField   *bText;
  GLLabel       *oldColor;
  GLLabel       *newColor;
  GLLabel       *swBox;
  GLLabel       *hsBox;
  GLLabel       *vBox;
  GLuint         hsvTex;
  GLuint         sliderTex;
  float          curH;
  float          curS;
  float          curV;
  bool           draggV;

  GLColorBox(char *title,int *r,int *g,int *b);
  ~GLColorBox();
  void ProcessMessage(GLComponent *src,int message);
  void Paint();
  void ManageEvent(SDL_Event *evt);

  void rgb_to_hsv( int ri,int gi,int bi, float *h,float *s,float *v);
  DWORD hsv_to_rgb( float h,float s,float v,bool swap=false );
  float get_red( DWORD c );
  float get_green( DWORD c );
  float get_blue( DWORD c );
  int   get_redi( DWORD c );
  int   get_greeni( DWORD c );
  int   get_bluei( DWORD c );
  void  paintBox(int x,int y,int w,int h);
  void  updateColor(int r,int g,int b);

};

#endif /* _GLCOLORBOXH_ */
