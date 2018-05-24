// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLINPUTBOXH_
#define _GLINPUTBOXH_

//#include <SDL_opengl.h>
#include "GLWindow.h"

class GLTextField;

class GLInputBox : private GLWindow {

public:
  // Display a modal dialog and return the entered string (NULL on cancel)
  static char *GetInput(const char *initMessage=NULL,char *label=NULL,char *title=NULL);

  char *rValue;
 
private:

  GLTextField *text;

  GLInputBox(const char *message,char *label,char *title);
  void ProcessMessage(GLComponent *src,int message);

};

#endif /* _GLINPUTBOXH_ */
