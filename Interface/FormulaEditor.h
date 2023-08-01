/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
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

  GLToggle  *sampleConvergenceTgl;
  GLButton    *recalcButton;
    GLButton    *convPlotterButton;
    GLButton		*moveUpButton;
  GLButton		*moveDownButton;
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
