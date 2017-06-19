#pragma once

#include "GLApp/GLWindow.h"
#include "GLApp\GLParser.h"
class GLButton;
class GLLabel;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLList;
class Worker;
#include <vector>
#include <string>

class FormulaEditor : public GLWindow {

public:

  // Construction
  FormulaEditor(Worker *work);
  void RebuildList();
  void Refresh();

  void ReEvaluate();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);
  

private:

  Worker	   *work;

  GLButton    *recalcButton;
  GLButton		*moveUpButton;
  GLButton		*moveDownButton;
  GLLabel     *l1;
  GLLabel     *descL;
  GLList      *formulaList;
  GLTitledPanel *panel1;
  GLTitledPanel *panel2;

  std::vector<std::string> userExpressions,userFormulaNames;

  void EnableDisableMoveButtons();

};
