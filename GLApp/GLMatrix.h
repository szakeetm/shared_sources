// Copyright (c) 2011 rubicon IT GmbH
#ifndef _GLMATRIXH_
#define _GLMATRIXH_

#include <SDL2/SDL_opengl.h>

class GLMatrix {

public:

  GLMatrix();

  void Init33(float _11,float _12,float _13,
              float _21,float _22,float _23,
              float _31,float _32,float _33);

  void Identity();
  void RotateX(float angle);
  void RotateY(float angle);
  void RotateZ(float angle);
  void Translate(float x,float y,float z);
  void Transpose();

  void Multiply(GLMatrix *m1,GLMatrix *m2);
  void Multiply(GLMatrix *m2);

  void TransformVec(float x,float y,float z,float w,float *rx,float *ry,float *rz,float *rw) const;

  GLfloat *GetGL();
  void LoadGL(GLfloat *m);
  void LoadGL(GLenum which);

  float _11;float _12;float _13;float _14;
  float _21;float _22;float _23;float _24;
  float _31;float _32;float _33;float _34;
  float _41;float _42;float _43;float _44;

};

#endif /* _GLMATRIXH_ */
