// Copyright (c) 2011 rubicon IT GmbH
#pragma once

#include "GLComponent.h"
#include <string>

class GLToggle : public GLComponent {

public:

  // Construction
  GLToggle(int compId,const char *text);

  // Component method
  int  GetState();
  void SetText(std::string txt);
  void SetState(int setState);
  void SetTextColor(int r,int g,int b);
  void SetEnabled(bool enable); //override GLComponent for text color change
  void AllowMixedState(bool setAllow);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  char text[512];
  int  state; // 0=>Unchecked 1=>Checked 2=>Mixed
  float rText;
  float gText;
  float bText;
  bool allowMixedState;

};