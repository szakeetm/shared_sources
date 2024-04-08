#ifndef _VERTEXCOORDINATESH_
#define _VERTEXCOORDINATESH_

#include "GLApp/GLWindow.h"

class GLTextField;
class GLButton;
class GLList;

class Worker;

class VertexCoordinates : public GLWindow {

public:

  // Construction
  VertexCoordinates();

  // Component method
  void Display(Worker *w);
  void Update();

  // Implementation
  void ProcessMessage(GLComponent *src,int message) override;

private:

  //void GetSelected();

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
