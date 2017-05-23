#pragma once
/*
  File:        FacetCoordinates.h
  Description: Facet coordinates window
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
class GLTextField;
class GLList;
class GLButton;

#include <vector>

class Worker;
class Facet;

struct line;

class FacetCoordinates : public GLWindow {

public:

  // Construction
  FacetCoordinates();

  // Component method
  void Display(Worker *w);
  void UpdateId(int vertexId);
  void UpdateFromSelection();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  void GetSelected();
  void InsertVertex(size_t rowId,size_t vertexId);
  void RemoveRow(size_t rowId);
  void RebuildList();
  void ApplyChanges();
  

  Worker       *worker;
  Facet        *selFacet;
  GLList       *facetListC;
  GLButton     *dismissButton;
  GLButton     *updateButton; //apply
  GLButton     *insertLastButton;
  GLButton     *insertBeforeButton;
  GLButton     *removePosButton;
  GLTextField  *insertIdText;
  GLButton      *setXbutton, *setYbutton, *setZbutton;

  std::vector<line> lines;

};