#pragma once
#include "GLContainer.h"
class GLMenu;
#include <string>

class GLWindow : public GLContainer {

public:

  // Construction
  GLWindow();
  ~GLWindow();

  // Window methods
  void GetBounds(int *x,int *y,int *w,int *h) const;
  void SetPosition(int x,int y);
  void SetClosable(bool closable);
  int  GetHeight() const;
  int  GetWidth() const;

    [[maybe_unused]] void GetClientArea(int *x,int *y,int *w,int *h);
  void SetTitle(const char *title);
  void SetTitle(const std::string& title);
  void SetBorder(bool b);
  void SetBackgroundColor(int r,int g,int b);
  void GetBackgroundColor(int *r,int *g,int *b) const;
  static bool IsCtrlDown();
  static bool IsShiftDown();
  static bool IsAltDown();
  static bool IsSpaceDown();
  static bool IsDkeyDown();
  static bool IsZkeyDown();
  static bool IsCapsLockOn();
  static bool IsTabDown();
  //int  GetModState();
  int  GetX(GLComponent *src,SDL_Event *evt) const;
  int  GetY(GLComponent *src,SDL_Event *evt);
  int  GetScreenX(GLComponent *src) const;
  int  GetScreenY(GLComponent *src);
  void DoModal();
  void SetVisible(bool visible);
  bool IsVisible() const;
  void SetResizable(bool sizable);
  void SetIconfiable(bool iconifiable);
  void SetMinimumSize(int width,int height);
  void Iconify(bool iconify);
  void Maximise(bool max);
  bool IsIconic() const;
  bool IsMaximized() const;
  void SetAnimatedFocus(bool animate);

  // Expert usage
  void Clip(GLComponent *src,int lMargin,int uMargin,int rMargin,int bMargin);
  void ClipRect(GLComponent *src,int x,int y,int width,int height);
  void ClipToWindow();
  void ClipWindowExtent();
  bool IsMoving() const;
  bool IsInComp(GLComponent *src,int mx,int my);
  void SetMaster(bool master);
  bool IsDragging();
  int  GetIconWidth() const;
  void PaintTitle(int width,int height);
  void PaintMenuBar();
  void UpdateOnResize();

  // Menu management
  void SetMenuBar(GLComponent *bar,int hBar=20);
  void AddMenu(GLMenu *menu);
    [[maybe_unused]] void RemoveMenu(GLMenu *menu);
  void ReassignMenu(GLMenu* menu, GLContainer* newParent);
  void CloseMenu();

  //Implementation
  void ProcessMessage(GLComponent *src,int message) override;
  void ManageEvent(SDL_Event *evt) override;
  virtual void ManageMenu(SDL_Event *evt);
  virtual void Paint();
  virtual void PaintMenu();
  virtual void SetBounds(int x,int y,int w,int h);
  void CancelDrag(SDL_Event *evt) override;
  virtual void DestroyComponents() {};

  

protected:

  // Coordinates (absolute)
  int  _width;
  int  _height;
  int  posX;
  int  posY;

private:

  int  GetUpMargin();
  bool IsInWindow(int mx,int my) const;
  bool IsInSysButton(SDL_Event *evt,int which_btn);
  void UpdateSize(int newWidht,int newHeight,int cursor);

  int  draggMode;
  int  mXOrg;
  int  mYOrg;
  char _title[128];
  char iconTitle[64];
  int  closeState;
  bool closable=true;
  int  maxState;
  int  iconState;
  bool _iconifiable;
  bool _iconified;
  bool maximized;
  bool border;
  bool animateFocus;
  int  rBack;
  int  gBack;
  int  bBack;
  bool isMaster;
  GLComponent *menuBar;
  bool visible;
  bool isResizable;
  int  minWidth;
  int  minHeight;
  int  orgWidth;
  int  orgHeight;
  GLContainer *menus;
  bool isModal;
  int  iconWidth;
  int  posXSave;
  int  posYSave;
  int  widthSave;
  int  heightSave;
  size_t lastClick;

};