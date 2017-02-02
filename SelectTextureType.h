#pragma once
#include "GLApp\GLWindow.h"

class Worker;
class Geometry;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLButton;

class SelectTextureType : public GLWindow {

public:
  // Display a modal dialog and return the code of the pressed button
  SelectTextureType(Worker *w);
  int  rCode;
  void ProcessMessage(GLComponent *src,int message);
private:
  Geometry     *geom;
  Worker	   *work;
  GLButton
	  *selectButton,
	  *addSelectButton,
	  *remSelectButton;
  GLTextField
	  *ratioText,
	  *ratioMinText,
	  *ratioMaxText;
  GLToggle
	  *ratioToggle,
	  *ratioMinMaxToggle,
	  *desorbToggle,
	  *absorbToggle,
	  *reflectToggle,
	  *transparentToggle,
	  *directionToggle;
  GLTitledPanel
	  *resolutionpanel,
	  *textureTypePanel;
};
