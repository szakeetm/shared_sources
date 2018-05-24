/*
Program:     MolFlow+
Description: Monte Carlo simulator for ultra-high vacuum
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
*/
#ifndef _GLCHARTOPTIONSH_
#define _GLCHARTOPTIONSH_

//#include "AxisPanel.h"
#include "..\GLTabWindow.h"

class GLTitledPanel;
class GLChart;
class GLComponent;
class GLToggle;
class GLCombo;
class AxisPanel;
class GLLabel;
class GLTextField;

class GLChartOptions : public GLTabWindow {

public:

  // Construction
  GLChartOptions(GLChart *chart);
  ~GLChartOptions();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void error(char *m);
  void commit();

  GLChart    *chart;
  GLButton   *closeBtn;

  // general panel
  GLTitledPanel *gLegendPanel;

  GLLabel      *generalLegendLabel;
  GLTextField *generalLegendText;

  GLToggle    *generalLabelVisibleCheck;

  GLTitledPanel *gColorFontPanel;

  GLLabel *generalFontHeaderLabel;
  GLLabel *generalFontHeaderSampleLabel;
  GLButton *generalFontHeaderBtn;

  GLLabel *generalFontLabelLabel;
  GLLabel *generalFontLabelSampleLabel;
  GLButton *generalFontLabelBtn;

  GLLabel *generalBackColorLabel;
  GLLabel *generalBackColorView;
  GLButton *generalBackColorBtn;

  GLTitledPanel *gGridPanel;

  GLCombo *generalGridCombo;

  GLCombo *generalLabelPCombo;
  GLLabel *generalLabelPLabel;

  GLCombo *generalGridStyleCombo;
  GLLabel *generalGridStyleLabel;
  GLTitledPanel *gMiscPanel;

  GLLabel *generalDurationLabel;
  GLTextField *generalDurationText;

  // Axis panel
  AxisPanel *y1Panel;
  AxisPanel *y2Panel;
  AxisPanel *xPanel;

};

#endif /* _GLCHARTOPTIONSH_ */
