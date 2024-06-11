// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLTOOLKITH_
#define _GLTOOLKITH_

#include "graphicsLibraries.h"
#include <string>
#include <optional>
#include <tuple>
#include "GLTypes.h"
#include "GLFont.h"
#include "GLMatrix.h"
#include "Vector.h"
//class GLFont2D;

// Dashed line style
#define DASHSTYLE_NONE      0
#define DASHSTYLE_DOT       1
#define DASHSTYLE_DASH      2
#define DASHSTYLE_LONG_DASH 3
#define DASHSTYLE_DASH_DOT  4

// Cursor type
#define CURSOR_DEFAULT 0
#define CURSOR_SIZE    1
#define CURSOR_SIZEH   2
#define CURSOR_SIZEHS  3
#define CURSOR_SIZEV   4
#define CURSOR_ZOOM    5
#define CURSOR_ZOOM2   6
#define CURSOR_TEXT    7
#define CURSOR_SELADD  8
#define CURSOR_SELDEL  9
#define CURSOR_HAND    10
#define CURSOR_ROTATE  11
#define CURSOR_VERTEX  12
#define CURSOR_VERTEX_ADD  13
#define CURSOR_VERTEX_CLR  14
#define CURSOR_BUSY  15
#define CURSOR_TRAJ  16

// Rouding work around (for glVertex2i)
#define _glVertex2i(x,y) glVertex2f((float)(x)-0.001f,(float)(y)+0.001f)
//#define _glVertex2i(x,y) glVertex2i(x,y)

struct ScreenSize {
	int width = 0;
	int height = 0;
	ScreenSize(int w, int h) : width(w), height(h) {};
};

struct ScreenCoord {
	ScreenCoord() = default;
	ScreenCoord(const int x_, const int y_) : x(x_), y(y_) {};
	int x;
	int y;
};

class GLToolkit {

public:

  // Utils functions
  static GLFont2D *GetDialogFont();
  static GLFont2D *GetDialogFontBold();
  static void GetScreenSize(int *width,int *height);
  static ScreenSize GetScreenSize();
  static void SetViewport(const int x,const int y,const int width,const int height);
  static void SetViewport(const GLVIEWPORT &v);
  static void SetMaterial(GLMATERIAL *mat);
  static void printGlError(GLenum glError);
  static std::optional<ScreenCoord> Get2DScreenCoord_currentMatrix(const Vector3d& p);
  static std::optional<ScreenCoord> Get2DScreenCoord_fast(const Vector3d& p, const GLMatrix& mvp, const GLVIEWPORT& viewPort);
  static void LookAt(const Vector3d& Eye, const Vector3d& camPos, const Vector3d& Up, const double handedness);
  static void PerspectiveLH(double fovy,double aspect,double zNear,double zFar);
  static float GetCamDistance(GLfloat *mView,double x,double y,double z); //unused
  static float GetVisibility(double x,double y,double z,double nx,double ny,double nz); //unused
  static void SetCursor(int cursor);
  static int  GetCursor();
  static void SetIcon32x32(const char *pngName);
  static void Log(const char *message);
  static char *GetLogs();
  static void ClearLogs();
  static void CheckGLErrors(const char *compname);

  // Drawing functions
  static void DrawBox(const int x,const int y,const int width,const int height,
	  const int r,const int g,const int b,const bool shadow=false,const bool iShadow=false,
	  const bool isEtched=false);
  static void DrawBorder(const int x,const int y,const int width,const int height,
	  const bool shadow,const bool iShadow,const bool isEtched);
  static void DrawStringInit();
  static void DrawStringRestore();
  static void DrawString(float x,float y,float z,const char *str,GLFont2D *fnt,int offx=0,int offy=0,const bool fast=true);
  static void DrawPoly(int lineWidth, int dashStyle, int r, int g, int b, int nbPoint, int *pointX, int *pointY,
                       bool useSmooth);
  static void DrawLumBitmap(int x,int y,int width,int height,BYTE *buffer);
  static void DrawCoordinateAxes(double vectorLength,double headSize=0.0,bool colored=false);
  static void DrawVector(double x1,double y1,double z1,double x2,double y2,double z2, const double headSize=0.0); //Auto-choose a normal and draw vector
  static void DrawVector(const Vector3d& start, const Vector3d& end, const Vector3d& normal, double headSize=0.0); //Draw vector with passed normal
  static void DrawButtonBack(const int x,const int y,const int width,
	  const int height,const int state);
  static void DrawSmallButton(int x,int y,int state);
  static void DrawTinyButton(int x,int y,int state);
  static void DrawToggle(int x,int y);
  static void DrawBar(int x,int y,int width,int height);
  static void DrawTextBack(const int x,const int y,const int width,
	  const int height,const int rBack,const int gBack,const int bBack);
  static void DrawVGradientBox(int x,int y,int width,int height,bool shadow=false,bool iShadow=false,bool isEtched=false);
  static void DrawHGradientBox(int x,int y,int width,int height,bool shadow=false,bool iShadow=false,bool isEtched=false);
  static void DrawHIGradientBox(int x,int y,int width,int height,bool shadow=false,bool iShadow=false,bool isEtched=false);
  static void DrawHScroll(int x,int y,int width,int height,int state);
  static void DrawVScroll(int x,int y,int width,int height,int state);
  static void Draw16x16(int x,int y,int xt,int yt);

  // Initialisation
  static bool RestoreDeviceObjects(const int width,const int height);
  static void InvalidateDeviceObjects();

  static void CopyTextToClipboard(const std::string& text);
  static std::string GetOSName();
  static std::tuple<GLMatrix, GLMatrix, GLMatrix, GLVIEWPORT> GetCurrentMatrices();
};

#endif /* _GLTOOLKITH_ */
