// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLPROGESSH_
#define _GLPROGESSH_

//#include <SDL_opengl.h>
#include "GLWindow.h"
#include <string>

class GLLabel;

class GLProgress : public GLWindow {

public:

  GLProgress(const char *message,const char *title);

  // Update progress bar (0 to 1)
  void SetProgress(double value);
  double GetProgress();
  void SetMessage(const char *msg, const bool& force=true);
  void SetMessage(const std::string& msg, const bool& force=true);
  void PositionComponents(); //Resizes window in cae of wide/multiline text

private:

  GLLabel   *progressBar; //rectangle that grows right
  GLLabel	*progressBarBackground; //rectangle background
  GLLabel   *percentLabel; //percent display in middle
  GLLabel	*progressStatus; //Showing what is being done
  size_t        progress; //percent, 0-100
  int        progressBarX,progressBarY,progressBarWidth,progressBarMaxWidth,progressBarHeight; //progressBar bounds
  Uint32     lastUpd; //update time to limit update to 2fps

  void ProcessMessage(GLComponent *src,int message) override;

};

#endif /* _GLPROGESSH_ */
