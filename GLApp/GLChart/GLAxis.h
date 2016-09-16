/*
  File:        GLAxis.h
  Description: 2D 'scientific oriented' chart component (Axis managenent)
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

#ifndef _GLCHARTAXISH_
#define _GLCHARTAXISH_

typedef struct {

  char *value;
  GLCDimension size;
  double pos;
  int offsetX;
  int offsetY;

} LabelInfo;

class GLAxis {

public:

  // Construction
  GLAxis(GLComponent *parent, int orientation);
  ~GLAxis();

  // Components method
  void     SetAxisColor(GLCColor c);
  GLCColor GetAxisColor();
  void   SetPercentScrollback(double d);
  double GetPercentScrollback();
  void   SetLabelFormat(int l);
  int    GetLabelFormat();
  void   SetGridVisible(BOOL b);
  void   SetFitXAxisToDisplayDuration(BOOL b);
  BOOL   IsFitXAxisToDisplayDuration();
  BOOL   IsGridVisible();
  void   SetDrawOpposite(BOOL b);
  BOOL   IsDrawOpposite();
  void   SetSubGridVisible(BOOL b);
  BOOL   IsSubGridVisible();
  void   SetGridStyle(int s);
  int    GetGridStyle();
  void   SetAnnotation(int a);
  int    GetAnnotation();
  void   SetVisible(BOOL b);
  BOOL   IsVisible();
  BOOL   IsZoomed();
  BOOL   IsXY();
  void   SetMinimum(double d);
  double GetMinimum();
  void   SetMaximum(double d);
  double GetMaximum();
  double GetMin();
  double GetMax();
  BOOL   IsAutoScale();
  void   SetAutoScale(BOOL b);
  int    GetScale();
  void   SetScale(int s);
  void   SetOrientation(int orientation);
  int    GetOrientation();
  void   Zoom(int x1, int x2);
  void   Unzoom();
  int    GetTick();
  double GetTickSpacing();
  void   SetTickSpacing(double spacing);
  void   SetTick(int s);
  void   SetTickLength(int lgth);
  int    GetTickLength();
  char  *GetName();
  void   SetName(char *s);
  void   SetPosition(int o);
  int    GetPosition();
  void   AddDataView(GLDataView *v);
  void   AddDataViewAt(int index,GLDataView *v);
  GLDataView *GetDataView(int index);
  void   RemoveDataView(GLDataView *v);
  BOOL   CheckRemoveDataView(GLDataView *v);
  void   ClearDataView();
  GLDataView **GetViews();
  int    GetViewNumber();
  void   SetInverted(BOOL i);
  BOOL   IsInverted();
  GLCRectangle GetBoundRect();
  char  *ToScientific(double d);
  char  *ToScientificInt(double d);
  char  *FormatValue(double vt, double prec);
  char  *FormatTimeValue(double vt);
  BOOL   IsHorizontal();
  void SetAxisDuration(double d);
  void ComputeXScale(GLDataView **views,int sz);
  int GetFontHeight();
  int GetLabelFontDimension();
  int GetFontOverWidth();
  int GetThickness();
  void MeasureAxis(int desiredWidth, int desiredHeight);
  GLCPoint transform(double x, double y, GLAxis *xAxis);
  SearchInfo *SearchNearest(int x, int y, GLAxis *xAxis);
  void computeLabels(double length);
  void DrawFast(GLCPoint lp, GLCPoint p, GLDataView *v);
  static void PaintMarker(GLCColor c,int mType, int mSize, int x, int y);
  static void DrawSampleLine(int x, int y, GLDataView *v);
  void PaintDataViews(GLAxis *xAxis, int xOrg, int yOrg);
  GLCColor ComputeMediumColor(GLCColor c1, GLCColor c2);
  void  PaintAxis(int x0, int y0, GLAxis *xAxis, int xOrg, int yOrg, GLCColor back,BOOL oppositeVisible);
  void  PaintAxisDirect(int x0, int y0,GLCColor back,int tr,int la);
  void  PaintAxisOpposite(int x0, int y0,GLCColor back,int tr,int la);
  void  PaintAxisOppositeDouble(int x0, int y0,GLCColor back,int tr,int la);
  BOOL  IsZeroAlwaysVisible();
  void  SetZeroAlwaysVisible(BOOL zeroAlwaysVisible);
  char *GetDateFormat();
  void  SetDateFormat (char *dateFormat);
  static void  Invalidate();
  static void  Revalidate();

private:

  int    getDV(GLDataView *v);
  BOOL   insideRect(GLCRectangle *r,GLCPoint *p);
  void   computeDateformat(int maxLab);
  char  *suppressZero(char *n);
  double computeHighTen(double d);
  double computeLowTen(double d);
  void   computeAutoScale();
  int    getLength();
  SearchInfo *searchNearestNormal(int x, int y, GLAxis *xAxis);
  SearchInfo *searchNearestXY(int x, int y, GLAxis *xAxis);
  void paintDataViewNormal(GLDataView *v, GLAxis *xAxis, int xOrg, int yOrg);
  void paintDataViewXY(GLDataView *v, GLDataView *w, GLAxis *xAxis, int xOrg, int yOrg);
  void paintYOutTicks(GLCColor c,int x0, double ys, int y0, int la, int tr,int off,BOOL grid);
  void paintXOutTicks(GLCColor c,int y0, double xs, int x0, int la, int tr,int off,BOOL grid);
  int  getTickShift(int width);
  int  getTickShiftOpposite(int width);
  void paintYTicks(GLCColor c,int i, int x0, double y, int la, int tr,int off,BOOL grid);
  void paintXTicks(GLCColor c,int i, int y0, double x, int la, int tr,int off,BOOL grid);
  int  computeBarWidth(GLDataView *v, GLAxis *xAxis);
  void paintDataViewBar(GLDataView *v,int barWidth,int y0,int x,int y);
  void paintDataViewPolyline(GLDataView *v,int nb,int yOrg,int *pointX,int *pointY);
  int  distance2(int x1, int y1, int x2, int y2);
  void paintBarBorder(int barWidth, int y0, int x, int y);
  void paintBar(int barWidth, GLCColor background, int fillStyle, int y0, int x, int y);
  void addLabel(char *lab, int w, int h, double pos,int offX=0,int offY=0);
  void clearLabel();
  void clip(int x,int y,int width,int height);
  static void drawLine(GLCColor c,int dash,int lWidth,int x1,int y1,int x2,int y2);
  static GLuint initMarker(char *name);
  static void paintMarkerTex(GLuint mTex,int x,int y,int width,int height,int r,int g,int b);

  BOOL visible;
  double min;
  double max;
  double minimum;
  double maximum;
  BOOL autoScale;
  int scale;
  GLCColor labelColor;
  int labelFormat;
  LabelInfo labels[MAX_VIEWS];
  int nbLabel;
  int orientation;  // Axis orientation/position
  int dOrientation; // Default orientation (cannot be _ORG)
  BOOL subtickVisible;
  GLCDimension csize;
  char name[256];
  int annotation;
  GLDataView *dataViews[MAX_VIEWS];
  int nbView;
  double ln10;
  BOOL gridVisible;
  BOOL subGridVisible;
  int gridStyle;
  GLCRectangle boundRect;
  BOOL lastAutoScate;
  BOOL isZoomed;
  double percentScrollback;
  double axisDuration;
  char *useFormat;
  double desiredPrec;
  BOOL drawOpposite;
  int tickLength;
  int subtickLength;
  int fontOverWidth;
  BOOL inverted;
  double tickStep;    // In pixel
  double minTickStep;
  int subTickStep; // 0 => NONE , -1 => Log step , 1.. => Linear step
  BOOL fitXAxisToDisplayDuration;
  
  BOOL zeroAlwaysVisible;
  BOOL  autoLabeling;

  char *dateFormat;

  GLComponent *parent;

};

#endif /* _GLCHARTAXISH_ */