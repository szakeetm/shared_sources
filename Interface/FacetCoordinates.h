
#pragma once


#include "GLApp/GLWindow.h"
class GLTextField;
class GLList;
class GLButton;

#include <vector>

class Worker;
class InterfaceFacet;

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
  void ProcessMessage(GLComponent *src,int message) override;

private:

  void GetSelected();
  void InsertVertex(size_t rowId,size_t vertexId);
  void RemoveRow(size_t rowId);
  void RebuildList();
  void ApplyChanges();
  

  Worker       *worker;
  InterfaceFacet        *selFacet;
  GLList       *facetListC;
  GLButton     *dismissButton;
  GLButton     *applyButton; //apply
  GLButton     *insertLastButton;
  GLButton     *insertBeforeButton;
  GLButton     *removePosButton;
  GLTextField  *insertIdText;
  GLButton      *setXbutton, *setYbutton, *setZbutton;

  std::vector<line> lines;

};