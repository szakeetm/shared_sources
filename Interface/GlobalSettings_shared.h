
#pragma once

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLGradient.h"
#include "Buffer_shared.h" //MAX_PROCESS macro

class Worker;
class GLList;

class GlobalSettingsBase : public GLWindow {

public:

    // Construction
    GlobalSettingsBase(Worker *w);
    void UpdateProcessList();

    // Implementation
    void ProcessMessage_shared(GLComponent *src,int message);
    void Display(Worker *w);
    void Update_shared();
    void UpdateDesLimitButtonText();
    virtual void ProcessMessage(GLComponent *src,int message) = 0;
    virtual void Update() = 0; //! implements software specific update steps on top of Update_shared
protected:

    void RestartProc();
    Worker      *worker;
    GLList      *processList;
    GLTitledPanel* processPanel;
    GLLabel* coreLabel;
    GLLabel* subProcLabel;
    GLButton    *restartButton;
    GLButton    *desLimitButton;
    GLTextField *nbProcText;
    GLTextField *autoSaveText;

    GLToggle      *lowFluxToggle;
    GLButton    *lowFluxInfo;
    GLTextField *cutoffText;

    int lastUpdate=0;

    GLToggle      *chkAntiAliasing;
    GLToggle      *chkWhiteBg;
    GLToggle		*leftHandedToggle;
    GLToggle      *chkSimuOnly;
    GLToggle      *chkCheckForUpdates;
    GLToggle      *chkCompressSavedFiles;
    GLButton    *applyButton;
    GLToggle *highlightSelectionToggle;

    GLToggle* highlightNonplanarToggle;
    GLToggle* useOldXMLFormat;
    GLToggle* prioToggle;
};
