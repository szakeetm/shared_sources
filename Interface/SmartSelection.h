#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;

class InterfaceGeometry;
class Worker;

class SmartSelection : public GLWindow {

public:

  // Construction
  SmartSelection(InterfaceGeometry *g,Worker *w);
  bool IsSmartSelection();
  double GetMaxAngle();
  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

  GLButton    *analyzeButton;
  GLLabel     *resultLabel;
  GLTextField *angleThreshold;
  GLToggle    *enableToggle;

  bool isRunning;
};

