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

/*
  File:        ScaleFacet.h
  Description: Mirror facet to plane dialog
*/
#ifndef _ScaleFacetH_
#define _ScaleFacetH_

#include "GLApp/GLWindow.h"

class GLButton;
class GLTextField;
class GLLabel;
class GLToggle;
class GLTitledPanel;

class InterfaceGeometry;
class Worker;

class ScaleFacet : public GLWindow {

public:
  // Construction
  ScaleFacet(InterfaceGeometry *interfGeom,Worker *work);
  void ProcessMessage(GLComponent *src,int message) override;

  // Implementation
private:

  void UpdateToggle(GLComponent *src);
  
  GLTitledPanel *iPanel;
  GLTitledPanel *sPanel;
  GLButton     *scaleButton;
  GLButton    *copyButton;
  GLButton    *cancelButton;
  GLButton	  *getSelFacetButton;
  GLToggle     *l1;
  GLToggle     *l2;
  GLToggle     *l3;
  GLTextField *xText;
  GLTextField *yText;
  GLTextField *zText;
  GLTextField *facetNumber;
  GLTextField *factorNumber;
  GLTextField *OnePerFactor;
  GLTextField *factorNumberX;
  GLTextField *factorNumberY;
  GLTextField *factorNumberZ;
  GLToggle    *uniform;
  GLToggle    *distort;

  int nbFacetS,invariantMode,scaleMode;

  InterfaceGeometry     *interfGeom;
  Worker	   *work;

};

#endif /* _ScaleFacetH_ */
