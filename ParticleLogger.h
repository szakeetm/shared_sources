#pragma once

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class Geometry;
class Worker;
class GLTitledPanel;
class ParticleLoggerItem;

class ParticleLogger : public GLWindow {

public:

  // Construction
  ParticleLogger(Geometry *geom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

  void UpdateMemoryEstimate();

  void UpdateStatus();

private:

  Geometry     *geom;
  Worker	   *work;

	GLLabel	*descriptionLabel;
	GLLabel	*label2;
	GLButton	*getSelectedFacetButton;
	GLTextField	*facetNumberTextbox;
	GLLabel	*label3;
	GLTextField	*maxRecordedTextbox;
	GLLabel	*memoryLabel;
	GLButton	*applyButton;
	GLLabel	*statusLabel;
	GLButton	*copyButton;
	GLButton	*exportButton;
	GLToggle	*enableCheckbox;
	GLTitledPanel	*logParamPanel;
	GLTitledPanel	*resultPanel;

	std::string ConvertLogToText(const size_t & nbRec, ParticleLoggerItem* log, const std::string & separator, std::ofstream* file=NULL);
	bool isRunning;
};


