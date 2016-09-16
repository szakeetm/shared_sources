/*
  File:        GLDataViewOptions.h
  Description: 2D 'scientific oriented' chart component (Dataview options dialog)
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

#ifndef _GLCHARTDVOPTIONSH_
#define _GLCHARTDVOPTIONSH_

#include "../GLSpinner.h"

class GLDataViewOptions : public GLTabWindow {

public:

  // Construction
  GLDataViewOptions::GLDataViewOptions(GLChart *chart);
  ~GLDataViewOptions();

  // Component methods
  void SetDataView(GLDataView *v);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void updateControls();
  void error(char *m);
  void commit();
  void setBounds(GLComponent *org,GLComponent *src,int x,int y,int w,int h);

  // Global
  GLDataView *dataView;
  GLChart    *chart;
  GLLabel    *nameLabel;
  GLButton   *closeBtn;

  // DataView general option panel
  GLLabel   *viewTypeLabel;
  GLCombo   *viewTypeCombo;

  GLLabel   *lineColorView;
  GLButton  *lineColorBtn;
  GLLabel   *lineColorLabel;

  GLLabel   *fillColorView;
  GLButton  *fillColorBtn;
  GLLabel   *fillColorLabel;

  GLLabel   *fillStyleLabel;
  GLCombo   *fillStyleCombo;

  GLLabel   *lineWidthLabel;
  GLSpinner *lineWidthSpinner;

  GLLabel *lineDashLabel;
  GLCombo *lineDashCombo;

  GLLabel *lineNameLabel;
  GLTextField *lineNameText;

  // Bar panel
  GLLabel   *barWidthLabel;
  GLSpinner *barWidthSpinner;

  GLLabel   *fillMethodLabel;
  GLCombo   *fillMethodCombo;

  // marker option panel
  GLLabel  *markerColorView;
  GLButton *markerColorBtn;
  GLLabel  *markerColorLabel;

  GLLabel   *markerSizeLabel;
  GLSpinner *markerSizeSpinner;

  GLLabel *markerStyleLabel;
  GLCombo *markerStyleCombo;

  GLToggle *labelVisibleCheck;

  //transformation panel
  GLLabel *transformHelpLabel;

  GLLabel *transformA0Label;
  GLTextField *transformA0Text;

  GLLabel *transformA1Label;
  GLTextField *transformA1Text;

  GLLabel *transformA2Label;
  GLTextField *transformA2Text;

  //Interpolation panel
  GLToggle          *noInterpBtn;
  GLToggle          *linearBtn;
  GLToggle          *cosineBtn;
  GLToggle          *cubicBtn;
  GLToggle          *hermiteBtn;
  GLSpinner         *stepSpinner;
  GLTextField       *tensionText;
  GLTextField       *biasText;

  //Smoothing panel
  GLToggle         *noSmoothBtn;
  GLToggle         *flatSmoothBtn;
  GLToggle         *triangularSmoothBtn;
  GLToggle         *gaussianSmoothBtn;
  GLSpinner        *neighborSpinner;
  GLTextField      *sigmaText;
  GLToggle         *noExtBtn;
  GLToggle         *flatExtBtn;
  GLToggle         *linearExtBtn;

  //Math panel
  GLToggle         *noMathBtn;
  GLToggle         *derivativeBtn;
  GLToggle         *integralBtn;
  GLToggle         *fftModBtn;
  GLToggle         *fftPhaseBtn;


};

#endif /* _GLCHARTDVOPTIONSH_ */
