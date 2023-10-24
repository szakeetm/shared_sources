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

#include "GLApp/GLComponent.h"
#include "GLApp/GLTypes.h"
#include "GeometryTypes.h"
#include "Vector.h"
#include <vector>

class Worker;
class GLButton;
class GLLabel;
class GLCombo;
class GLOverlayLabel;

enum DragMode : int {
	DragNone,
	DragSelectFacet,
	DragRotate,
	DragZoom,
	DragPan,
	DragSelectVertex
#ifdef  SYNRAD
	, DragSelectTrajectory
#endif
};

enum CursorMode : int {
	CursorSelectFacet,
	CursorZoom,
	CursorPan,
	CursorSelectVertex,
	#ifdef  SYNRAD
	,CursorSelectTrajectory
	#endif
};





#define MSG_GEOMVIEWER_MAXIMISE MSG_USER + 1
#define MSG_GEOMVIEWER_SELECT   MSG_USER + 2

#define FOV_ANGLE 45.0

struct ScreenshotStatus{
	int requested=0; //0=no request, 1=waiting for area selection, 2=take screenshot on next viewer paint
	std::string fileName;
	int x, y, w, h; //Screenshotarea
} ;

class GeometryViewer : public GLComponent {

public:

  // Construction
  GeometryViewer(int id);

  void UpdateLabelColors();

  // Component method
  void ToOrigo();
  void SetWorker(Worker *s);
  void SetProjection(ProjectionMode projMode);
  void ToTopView();
  void ToSideView();
  void ToFrontView();
  bool SelectionChanged();
  bool IsDragging() override;
  CameraView GetCurrentView();
  void  SetCurrentView(CameraView v);
  bool IsSelected();
  void SetSelected(bool s);

  // Implementation
  void Paint() override;
  void ManageEvent(SDL_Event *evt) override;
  void SetBounds(int x,int y,int width,int height) override;
  void ProcessMessage(GLComponent *src,int message) override;
  void SetFocus(bool focus) override;

  void SelectCoplanar(double tolerance); //launcher function to get viewport parameters
  void UpdateMatrix();
  Plane GetCameraPlane();
  void RequestScreenshot(std::string fileName, int x,int y,int w,int h);

  // Flag view
  bool showIndex = false;
  bool showNormal = false;
  bool showRule = true;
  bool showUV = false;
  bool showLeak = false;
  bool showHit = false;
  bool showLine = false;
  bool showVolume = false;
  bool showTexture = false;
  bool showFacetId = false;
  bool showVertexId = false;
  VolumeRenderMode  volumeRenderMode = VolumeRenderMode :: FrontAndBack;
  bool showFilter = false;
  // bool showColormap;
  bool showTP = true;
  bool showHiddenFacet = false;
  bool showHiddenVertex = true;
  bool showMesh = false;
  bool bigDots = true;
  bool showDir = true;
  bool autoScaleOn = false;
  int  hideLot = 500;

  #ifdef  MOLFLOW
  bool showTime = false;
  #endif

  #ifdef  SYNRAD
  bool shadeLines = true;
  size_t dispNumTraj = 1000;  // displayed number of trajectory points
  #endif
  
  size_t dispNumHits=2048; // displayed number of lines and hits
  size_t dispNumLeaks=2048; // displayed number of leaks
  double transStep=1.0;  // translation step
  double angleStep=0.005;  // angle step
  
  GLLabel       *facetSearchState;

private:

  double ToDeg(double radians);
  void DrawIndex();
  void DrawCoordinateAxes();
  void DrawNormal();
  void DrawUV();
  void DrawFacetId();
  void DrawLeak();
  void DrawLinesAndHits();
  void Zoom();
  void UpdateMouseCursor(CursorMode mode);
  void TranslateScale(double diff);
  void PaintCompAndBorder();
  void PaintSelectedVertices(bool hiddenVertex);
  void AutoScale(bool reUpdateMouseCursor=true);
  void ComputeBB(/*bool getAll*/);
  void UpdateLight();
  void Screenshot();

  //void DrawBB();
  //void DrawBB(AABBNODE *node);

  Worker *work = nullptr;

  // Toolbar
  GLLabel       *toolBack;
  GLButton      *frontBtn;
  GLButton      *topBtn;
  GLButton      *sideBtn;
  GLCombo       *projCombo;
  GLButton      *zoomBtn;
  GLButton      *autoBtn;
  GLButton      *selBtn;
  GLButton      *selVxBtn;
  GLButton      *sysBtn;
  GLButton      *handBtn;
  GLLabel       *coordLab;

  
  GLLabel       *capsLockLabel;
  GLLabel       *hideLotlabel;
  GLLabel		*screenshotLabel;
  GLLabel		*selectLabel;
  GLLabel		*rotateLabel;
  GLLabel		*panLabel;
  GLLabel		*tabLabel;
  GLLabel		*nonPlanarLabel;

  #if defined(MOLFLOW)
  GLOverlayLabel *timeLabel;
  #endif
  
  #if defined(SYNRAD)
  GLButton      *selTrajBtn;
  #endif
  
  // Viewer mode
  DragMode      dragMode = DragMode::DragNone;
  CursorMode      cursorMode = CursorMode::CursorSelectFacet;
  bool     selected=false;

  // View parameters
  CameraView    view;

  // Camera<->mouse motions
  int      mXOrg;
  int      mYOrg;    
  double   camDistInc = 1.0; //zoom speed

  // Transformed BB
  double   xMin;
  double   xMax;
  double   yMin;
  double   yMax;
  double   zNear;
  double   zFar;

  Vector3d camDir;     // Camera basis (ProjectionMode::Perspective)
  Vector3d camLeft;    // Camera basis (ProjectionMode::Perspective)
  Vector3d camUp;      // Camera basis (ProjectionMode::Perspective)

  double   vectorLength = 5.0; //Coordinate axis default vector length
  double   headSize = .1 * vectorLength; //Default vectorhead size (10% arrow head length), one global value to match N,U,V,axis vector heads

  // Rectangle selection
  int      selX1=0;
  int      selY1=0;
  int      selX2=0;
  int      selY2=0;

  // Selection change
  bool selectionChange = false;
  ScreenshotStatus screenshotStatus;

  // SDL/OpenGL stuff
  GLfloat matView[16];
  GLfloat matProj[16];
  GLMATERIAL greenMaterial;
  GLMATERIAL blueMaterial;
};
