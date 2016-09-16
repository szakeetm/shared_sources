/*
  File:        GeometryViewer.h
  Description: Geometry 3D Viewer components
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY
  Copyright:   E.S.R.F / CERN

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "GLApp/GLComponent.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLLabel.h"
#include "GLApp\GLGradient.h"
#include "Worker.h"

#ifndef _GEOMETRYVIEWERH_
#define _GEOMETRYVIEWERH_

#define MAX_VIEWER  4

#define DRAGG_NONE   0
#define DRAGG_SELECT 1
#define DRAGG_ROTATE 2
#define DRAGG_ZOOM   3
#define DRAGG_MOVE   4
#define DRAGG_SELECTVERTEX 5
#ifdef  SYNRAD
#define DRAGG_SELECTTRAJ 6
#endif

#define MODE_SELECT  0
#define MODE_ZOOM    1
#define MODE_MOVE    2
#define MODE_SELECTVERTEX 3
#ifdef  SYNRAD
#define MODE_SELECTTRAJ 4
#endif

#define SHOW_FRONTANDBACK 0
#define SHOW_FRONT        1
#define SHOW_BACK         2

#define PERSPECTIVE_PROJ  0
#define ORTHOGRAPHIC_PROJ 1

#define XYZ_NONE          0
#define XYZ_TOP           1
#define XYZ_SIDE          2
#define XYZ_FRONT         3

#define MSG_GEOMVIEWER_MAXIMISE MSG_USER + 1
#define MSG_GEOMVIEWER_SELECT   MSG_USER + 2

#define FOV_ANGLE 45.0

// Definition of a view. Note: all basis are left handed

typedef struct {

  char    *name;       // View name

  int      projMode;   // Projection type
  double   camAngleOx; // Spheric coordinates
  double   camAngleOy; // Spheric coordinates

  double   camAngleOz;

  double   camDist;    // Camera distance (or zoom in orthographic)

  double   lightAngleOx; //Light direction
  double   lightAngleOy; //Light direction

  VERTEX3D camOffset;  // Camera target offset
  int      performXY;  // Draw x,y,z coordinates when aligned with axis and orthographic

  double   vLeft;      // Viewport in 2D proj space (used for orthographic autoscaling)
  double   vRight;     // Viewport in 2D proj space (used for orthographic autoscaling)
  double   vTop;       // Viewport in 2D proj space (used for orthographic autoscaling)
  double   vBottom;    // Viewport in 2D proj space (used for orthographic autoscaling)

} AVIEW;

typedef struct {

  char    *name;       // Selection name

  int      nbSel;   
  int      *selection; // List of facets

} ASELECTION;

class GeometryViewer : public GLComponent {

public:

  // Construction
  GeometryViewer(int id);

  // Component method
  void ToOrigo();
  void SetWorker(Worker *s);
  void SetProjection(int mode);
  void ToTopView();
  void ToSideView();
  void ToFrontView();
  BOOL SelectionChanged();
  BOOL IsDragging();
  AVIEW GetCurrentView();
  void  SetCurrentView(AVIEW v);
  BOOL IsSelected();
  void SetSelected(BOOL s);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);
  void SetBounds(int x,int y,int width,int height);
  void ProcessMessage(GLComponent *src,int message);
  void SetFocus(BOOL focus);

  void SelectCoplanar(double tolerance); //launcher function to get viewport parameters
  void UpdateMatrix();

  // Flag view
  BOOL showIndex;
  BOOL showVertex;
  BOOL showNormal;
  BOOL showRule;
  BOOL showUV;
  BOOL showLeak;
  BOOL showHit;
  BOOL showLine;
  BOOL showVolume;
  BOOL showTexture;
  int  showBack;
  BOOL showFilter;
  BOOL showColormap;
  BOOL showTP;
  BOOL showHidden;
  BOOL showHiddenVertex;
  BOOL showMesh;
  BOOL bigDots;
  BOOL showDir;
  BOOL autoScaleOn;
  #ifdef  MOLFLOW
  BOOL showTime;
  #endif

  #ifdef  SYNRAD
  BOOL shadeLines;
  int dispNumTraj;  // displayed number of trajectory points
  #endif
  
  int dispNumHits; // displayed number of lines and hits
  int dispNumLeaks; // displayed number of leaks
  double transStep;  // translation step
  double angleStep;  // angle step
  
  GLLabel       *facetSearchState;

private:

  double ToDeg(double radians);
  void DrawIndex();
  void DrawRule();
  void DrawNormal();
  void DrawUV();
  void DrawLeak();
  void DrawLinesAndHits();
  void Zoom();
  void UpdateMouseCursor(int mode);
  void TranslateScale(double diff);
  void PaintCompAndBorder();
  void PaintSelectedVertices(BOOL hiddenVertex);
  void AutoScale(BOOL reUpdateMouseCursor=TRUE);
  void ComputeBB(BOOL getAll);
  void UpdateLight();

  //void DrawBB();
  //void DrawBB(AABBNODE *node);

  Worker *work;

  // Toolbar
  GLLabel       *toolBack;
  GLButton      *frontBtn;
  GLButton      *topBtn;
  GLButton      *sideBtn;
  GLCombo       *projCombo;
  GLLabel       *capsLockLabel;
  GLButton      *zoomBtn;
  GLButton      *autoBtn;
  GLButton      *selBtn;
  GLButton      *selVxBtn;
  GLButton      *sysBtn;
  GLButton      *handBtn;
  GLLabel       *coordLab;

  #ifdef MOLFLOW
  GLOverlayLabel *timeLabel;
  #endif
  
  #ifdef SYNRAD
  GLButton      *selTrajBtn;
  #endif
  
  // Viewer mode
  int      draggMode;
  int      mode;
  BOOL     selected;

  // View parameters
  AVIEW    view;

  // Camera<->mouse motions
  int      mXOrg;
  int      mYOrg;    
  double   camDistInc;

  // Transformed BB
  double   xMin;
  double   xMax;
  double   yMin;
  double   yMax;
  double   zNear;
  double   zFar;

  VERTEX3D camDir;     // Camera basis (PERSPECTIVE_PROJ)
  VERTEX3D camLeft;    // Camera basis (PERSPECTIVE_PROJ)
  VERTEX3D camUp;      // Camera basis (PERSPECTIVE_PROJ)

  double   vectorLength;
  double   arrowLength;

  // Rectangle selection
  int      selX1;
  int      selY1;
  int      selX2;
  int      selY2;

  // Selection change
  BOOL selectionChange;

  // SDL/OpenGL stuff
  GLfloat matView[16];
  GLfloat matProj[16];
  GLMATERIAL greenMaterial;
  GLMATERIAL blueMaterial;

};

#endif /* _GEOMETRYVIEWERH_ */
