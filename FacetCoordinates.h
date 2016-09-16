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
#include "GLApp/GLTextField.h"
#include "GLApp/GLList.h"
#include "GLApp/GLButton.h"
#include "Worker.h"

#ifndef _FACETCOORDINATESH_
#define _FACETCOORDINATESH_

struct line {
	  int vertexId;
	  VERTEX3D coord;
  };

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
  void InsertVertex(int rowId,int vertexId);
  void RemoveRow(int rowId);
  void setBounds(GLComponent *org,GLComponent *src,int x,int y,int w,int h);
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

#endif /* _FACETCOORDINATESH_ */
