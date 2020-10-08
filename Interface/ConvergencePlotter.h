/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#ifndef _CONVERGENCEPLOTTERH_
#define _CONVERGENCEPLOTTERH_

#include "GLApp/GLWindow.h"
#include "GLApp/GLChart/GLChartConst.h"
#include <vector>
#include <map>

class GLChart;
class GLLabel;
class GLCombo;
class GLButton;
class GLParser;
class GLDataView;
class GLToggle;
class GLTextField;
class Worker;
class Geometry;

class ConvergencePlotter : public GLWindow {

public:

  // Construction
  ConvergencePlotter(Worker *appWorker, std::vector<GLParser *> *formulaPtr,
                     std::vector<std::vector<std::pair<size_t, double>>> *convValuesPtr);

  // Component method
  void Display(Worker *w);
  void Refresh();
  void Update(float appTime);
  void Reset();

    // Implementation
  void ProcessMessage(GLComponent *src,int message) override;
  void SetBounds(int x,int y,int w,int h);
  int addView(int formulaId);
  std::vector<int> GetViews();
  void SetViews(const std::vector<int> &updatedViews);
  bool IsLogScaled();
  void SetLogScaled(bool logScale);
  void SetWorker(Worker *w);
  void UpdateVector();
    void ResetData();

private:  
  int remView(int formulaId);
  void refreshViews();
  void plot();
    void pruneEveryN(size_t everyN);
    void pruneFirstN(size_t n);
  Worker      *worker;
  GLButton    *dismissButton;
  GLChart     *chart;
  GLCombo     *profCombo;
  //GLToggle    *showAllMoments;

    GLButton    *pruneEveryNButton;
    GLButton    *pruneFirstNButton;
    GLButton    *addButton;
  GLButton    *removeButton;
  GLButton    *removeAllButton;
  GLTextField *formulaText;
  GLButton    *formulaBtn;
  GLToggle    *logYToggle;

    GLToggle    *colorToggle;
    GLLabel    *fixedLineWidthText;
    GLButton    *fixedLineWidthButton;
    GLTextField    *fixedLineWidthField;


    GLDataView  *views[MAX_VIEWS];
  int          nbView;
  float        lastUpdate;

  std::vector<GLParser*>* formulas_vecPtr;
  std::vector<std::vector<std::pair<size_t,double>>>* convValues_vecPtr;

};

#endif /* _CONVERGENCEPLOTTERH_ */
