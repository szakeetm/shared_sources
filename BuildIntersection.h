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
class Geometry;
class Worker;

class BuildIntersection : public GLWindow {

public:
  // Construction
  BuildIntersection(Geometry *geom,Worker *work);
  ~BuildIntersection();
  void ProcessMessage(GLComponent *src,int message);
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

  Geometry     *geom;
  Worker	   *work;

};
