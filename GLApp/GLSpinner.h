/*
Program:     MolFlow+
Description: Monte Carlo simulator for ultra-high vacuum
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
*/
#ifndef _GLSPINNERH_
#define _GLSPINNERH_

#include "GLComponent.h"

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
