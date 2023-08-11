/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
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

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/
#pragma once
/*
  File:        BuildIntersection.h
  Description: Build intersection dialog
*/

#include "GLApp/GLWindow.h"
//#include "GLApp/GLButton.h"
//#include "GLApp/GLLabel.h"
class GLButton;
class GLLabel;

//#include "Geometry_shared.h"
//#include "Worker.h"
class InterfaceGeometry;
class Worker;

class BuildIntersection : public GLWindow {

public:
  // Construction
  BuildIntersection(InterfaceGeometry *interfGeom,Worker *work);
  ~BuildIntersection();
  void ProcessMessage(GLComponent *src,int message) override;
  void ClearUndoFacets();

  // Implementation
private:
  
  GLLabel	*label1;
  /*GLButton	*XZplaneButton;
  GLButton	*YZplaneButton;
  GLButton	*XYplaneButton;
  GLLabel	*label4;
  GLLabel	*label3;
  GLLabel	*label2;*/
  GLLabel	*resultLabel;
  GLButton	*buildButton;
  GLButton	*undoButton;

  std::vector<DeletedFacet> deletedFacetList;
  size_t nbFacet, nbCreated;

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

};
