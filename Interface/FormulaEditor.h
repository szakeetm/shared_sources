
#pragma once

#include "GLApp/GLWindow.h"
#include "GLApp/GLFormula.h"
class GLButton;
class GLLabel;
class GLTextField;
class GLToggle;
class GLTitledPanel;
class GLList;
class Worker;
struct Formulas;

#include <vector>
#include <string>
#include <memory>

class FormulaEditor : public GLWindow {

public:

  // Construction
  FormulaEditor(Worker *w, std::shared_ptr<Formulas> formulas);

  void RebuildList();
  void Refresh();
  void UpdateValues();

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

  void SetBounds(int x, int y, int w, int h) override;
  

private:

  Worker	   *work;

  GLToggle  *recordConvergenceToggle;
  GLToggle* autoUpdateToggle;
  GLButton    *recalcButton;
    GLButton    *convPlotterButton;
    GLButton		*moveUpButton;
  GLButton		*moveDownButton;
  GLButton* copyAllButton;
  GLLabel     *descL;
  GLList      *formulaList;
  GLTitledPanel *panel1;
  GLTitledPanel *panel2;

  std::vector<std::string> userExpressions,userFormulaNames;
  std::vector<double> columnRatios;

  void EnableDisableMoveButtons();

public:
    std::shared_ptr<Formulas> appFormulas;
};
