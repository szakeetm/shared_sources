#pragma once
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
class GLButton;
class GLLabel;
class GLTextField;
class GLParser;

class FormulaSettings : public GLWindow {

public:

  // Construction
  FormulaSettings();

  // Components method
  void Update(GLParser *f,int id);

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void DisplayError(GLParser *f);

  GLLabel     *nameL;
  GLTextField *nameT;
  GLLabel     *exprL;
  GLTextField *exprT;
  GLLabel     *descL;

  GLButton    *applyButton;
  GLButton    *deleteButton;
  GLButton    *cancelButton;

  int formulaId;

};