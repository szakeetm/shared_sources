/*
  File:        VertexCoordinates.h
  Description: Vertex coordinates window
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY / M ADY
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
#include "GLApp/GLtextField.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLList.h"
#include "GLApp/GLButton.h"
#include "Worker.h"

#ifndef _VERTEXCOORDINATESH_
#define _VERTEXCOORDINATESH_

class VertexCoordinates : public GLWindow {

public:

  // Construction
  VertexCoordinates();

  // Component method
  void Display(Worker *w);
  void Update();

  // Implementation
  void ProcessMessage(GLComponent *src,int message);

private:

  //void GetSelected();
  void setBounds(GLComponent *org,GLComponent *src,int x,int y,int w,int h);

  Worker       *worker;
  //Facet        *selFacet;
  GLList       *vertexListC;
  GLButton     *dismissButton;
  GLButton     *updateButton;
  //GLButton     *insert1Button;
  //GLButton     *insert2Button;
  //GLButton     *removeButton;
  //GLTextField  *insertPosText;
  GLButton      *setXbutton, *setYbutton, *setZbutton;
};

#endif /* _VertexCoordinatesH_ */
