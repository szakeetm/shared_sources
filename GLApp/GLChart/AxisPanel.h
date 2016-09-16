/*
  File:        AxisPanel.h
  Description: 2D 'scientific oriented' chart component (Axis settings dialog)
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

#ifndef _GLCHARTAXISPANELH_
#define _GLCHARTAXISPANELH_

#include "../GLTabWindow.h"
#include "../GLTitledPanel.h"
#include "../GLButton.h"
#include "../GLCombo.h"
#include "../GLToggle.h"
#include "../GLLabel.h"
#include "../GLTextField.h"

class AxisPanel {

public:

   AxisPanel(GLAxis *a,int axisType,GLChart *parentChart);
   void AddToPanel(GLTabWindow *parent,int pIdx);
   void ProcessMessage(GLComponent *src,int message);

private:

  void commit();
  void error(char *m);
  void setBounds(GLComponent *org,GLComponent *src,int x,int y,int w,int h);

  GLAxis  *pAxis;
  GLChart *pChart;
  int     type;

  GLTitledPanel  *scalePanel;
  GLTitledPanel  *settingPanel;

  GLLabel     *MinLabel;
  GLTextField *MinText;
  GLLabel     *MaxLabel;
  GLTextField *MaxText;
  GLToggle    *AutoScaleCheck;

  GLLabel    *ScaleLabel;
  GLCombo    *ScaleCombo;
  GLToggle   *SubGridCheck;
  GLToggle   *VisibleCheck;
  GLToggle   *OppositeCheck;

  GLCombo    *FormatCombo;
  GLLabel    *FormatLabel;

  GLLabel     *TitleLabel;
  GLTextField *TitleText;

  GLLabel     *ColorLabel;
  GLLabel     *ColorView;
  GLButton    *ColorBtn;

  GLLabel     *PositionLabel;
  GLCombo     *PositionCombo;

};

#endif /* _GLCHARTAXISPANELH_ */
