
#pragma once

#include <vector>
#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class InterfaceGeometry;
class Worker;
class GLTitledPanel;
class ParticleLoggerItem;

class ParticleLogger : public GLWindow {

public:

  // Construction
  ParticleLogger(InterfaceGeometry *interfGeom,Worker *work);

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

  void UpdateMemoryEstimate();

  void UpdateStatus();

private:

  InterfaceGeometry     *interfGeom;
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

	std::string ConvertLogToText(const std::vector<ParticleLoggerItem> &log, const std::string &separator,
                                 std::ofstream *targetFile);
	bool isRunning;
};


