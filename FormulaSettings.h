/*
  File:        FormulaSettings.h
  Description: Formula edition dialog
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

#include "GLApp/GLWindow.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLParser.h"

#ifndef _FORMULASETTINGSH_
#define _FORMULASETTINGSH_

class FormulaSettings : public GLWindow {

public:

  // Construction
  FormulaSettings();

  // Components method
  BOOL EditFormula(GLParser *f);
  GLParser *NewFormula();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void DisplayError(GLParser *f);

  GLLabel     *nameL;
  GLTextField *nameT;
  GLLabel     *exprL;
  GLTextField *exprT;
  GLLabel     *descL;

  GLButton    *createButton;
  GLButton    *deleteButton;
  GLButton    *cancelButton;

  int rCode;

};

#endif /* _FORMULASETTINGSH_ */
