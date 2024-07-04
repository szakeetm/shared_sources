
#pragma once

//#include "GLApp.h"
#include "GLWindow.h"
class GLComponent;
class TabbedBar;

typedef struct {

  char         *name;
  int           width;
  GLComponent **comp;
  int           nbComp;
  bool          selected;

} APANEL;

class GLTabWindow : public GLWindow {

public:

  // Construction
  GLTabWindow();
  ~GLTabWindow();

  // Add/Remove components to this windows
  void Add(int panel,GLComponent *comp);
  void SetPanelNumber(int numP);
  void SetPanelName(int idx,const char *name);
  void Clear();
  void UpdateBar();
  void SetTextColor(int r,int g,int b);
  int GetSelectedTabIndex();

  //Overrides
  void SetBounds(int x,int y,int w,int h) override;
  void ProcessMessage(GLComponent *src,int message) override;

private:

  void showHide();

  APANEL *panels;
  int nbPanel;
  TabbedBar *bar;

};

