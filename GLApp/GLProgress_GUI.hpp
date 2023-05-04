#pragma once

//#include <SDL_opengl.h>
#include <Helper/GLProgress_abstract.hpp>
#include "GLWindow.h"

class GLLabel;

class GLProgress_GUI : public GLProgress_Abstract , public GLWindow {

public:

	GLProgress_GUI(const std::string& message,const std::string& title);

	// Update progress bar (0 to 1)
	void SetProgress(const double value) override;
	void SetMessage(const std::string& msg, const bool force=true);
	void PositionComponents(); //Resizes window in cae of wide/multiline text

private:

  GLLabel   *progressBar; //rectangle that grows right
  GLLabel	*progressBarBackground; //rectangle background
  GLLabel   *percentLabel; //percent display in middle
  GLLabel	*progressStatus; //Showing what is being done
  int        progressBarX,progressBarY,progressBarWidth,progressBarMaxWidth,progressBarHeight; //progressBar bounds
  Uint32     lastUpd; //update time to limit update to 2fps

  void ProcessMessage(GLComponent *src,int message) override;

};
