/*
  File:        GLChartOptions.h
  Description: 2D 'scientific oriented' chart component (Chart options dialog)
               C++ port of fr.esrf.tangoatk.widget.util.chart
  Author:      J-L PONS (2007)

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

#include "AxisPanel.h"

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
  void setBounds(GLComponent *org,GLComponent *src,int x,int y,int w,int h);

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
