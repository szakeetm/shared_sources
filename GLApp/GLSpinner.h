/*
  File:        GLSpinner.h
  Description: Spinner class (SDL/OpenGL OpenGL application framework)
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
#include "GLComponent.h"

#ifndef _GLSPINNERH_
#define _GLSPINNERH_

class GLSpinner : public GLComponent {

public:

  // Construction
  GLSpinner(int compId);

  // Components method
  void SetValue(double value);
  double GetValue();
  void SetMinMax(double min,double max);
  void SetIncrement(double inc);
  void SetFormat(char *format);

  // Implementation
  void Paint();
  void ManageEvent(SDL_Event *evt);

private:

  double value;
  double increment;
  double min;
  double max;
  char format[64];
  int  state;

};

#endif /* _GLSPINNERH_ */
