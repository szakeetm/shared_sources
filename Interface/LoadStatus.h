
#pragma once

#include "GLApp/GLWindow.h"
#include "SimulationManager.h" //loadstatus_abstract

class Worker;
class GLButton;
class GLLabel;
class GLTitledPanel;
class GLList;

class LoadStatus : public GLWindow, public LoadStatus_abstract{
	//Can display subprocess status when Update() called and request abort when Stop pressed
public:

  // Construction
  LoadStatus(Worker *w);
  void EnableStopButton() override;
  void RefreshNbProcess();
  void Update() override;

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

  GLList      *processList;
  GLLabel     *memInfoLabel;
  GLButton    *cancelButton;
  Worker      *worker;

  Uint32 lastUpd = 0;
  
};
