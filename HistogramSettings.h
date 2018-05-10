#pragma once

#include "GLApp/GLWindow.h"
#include <vector>

class GLWindow;
class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;

class Geometry;
class Worker;

class HistogramSettings : public GLWindow {

public:

  // Construction
  HistogramSettings();
  void Reposition();

  // Component methods
  void SetGeometry(Geometry *s,Worker *w);
  void Refresh(const std::vector<size_t>& selectedFacetIds);
  bool Apply();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  Geometry     *geom;
  Worker	   *work;

  //GLButton    *applyButton;
  GLTextField *distanceLimit;
  GLTextField *hitLimit;
  GLTextField *hitBinSize;
  GLToggle *recordToggle;
  GLLabel  *memoryEstimateLabel;

  size_t nbFacetSelected;

};