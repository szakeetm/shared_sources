
#ifndef _GLRECOVERYDIALOGH_
#define _GLRECOVERYDIALOGH_

//#include <SDL_opengl.h>
#include "GLApp/GLWindow.h"

// Buttons
#define GLDLG_LOAD		0x0001
#define GLDLG_SKIP		0x0002
#define GLDLG_DELETE	0x0008
#define GLDLG_CANCEL_R    0x0016

// Icons
#define GLDLG_ICONNONE    0
#define GLDLG_ICONERROR   1
#define GLDLG_ICONWARNING 2
#define GLDLG_ICONINFO    3

class RecoveryDialog : private GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  static int Display(const char *message,const char *title=NULL,int mode=GLDLG_LOAD|GLDLG_SKIP|GLDLG_DELETE,int icon=GLDLG_ICONINFO);
  int  rCode;

private:
	RecoveryDialog(const char *message, const char *title, int mode, int icon);
  void ProcessMessage(GLComponent *src,int message) override;

};

#endif
