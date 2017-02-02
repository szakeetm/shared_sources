#pragma once
#include "Vector.h"

typedef struct {

  Vector2d  *pts;   // Array of 2D vertex
  int        nbPts; // Number of vertex
  double     sign;  // Polygon orientation

} POLYGON;

typedef struct  {

  Vector2d  p;       // Vertex coordinates
  int       mark;    // Cycle detection (0=>not processed, 1=>processed)
  int       isStart; // Possible starting point

  int       nbOut;  // Number of outgoing arc
  int       nbIn;   // Number of incoming arc
  int       VI[2];  // Tangent point detection
  int       VO[2];  // Tangent point detection

} POLYVERTEX;

typedef struct {

  int i1;  // Node 1 index
  int i2;  // Node 2 index
  int s;   // Source polygon (tangent point detection)

} POLYARC;

typedef struct {

  int         nbNode;  // Number of node
  POLYVERTEX *nodes;   // Nodes
  int         nbArc;   // Number of arc
  POLYARC    *arcs;    // Arcs

} POLYGRAPH;

int   IsConvex(POLYGON *p,int idx);
int   IsInsideTri(Vector2d *p,Vector2d *p1,Vector2d *p2,Vector2d *p3);
int   ContainsConcave(POLYGON *p,int i1,int i2,int i3);
int   EmptyTriangle(POLYGON *p,int i1,int i2,int i3,Vector2d *center);
int   IsInPoly(double u,double v,Vector2d *pts,int nbPts);
int   IsOnPolyEdge(const double & u, const double & v, Vector2d * pts, const int & nbPts, const double & tolerance);
int   IsOnSection(const double & u, const double & v, const double & baseU, const double & baseV, const double & targetU, const double & targetV, const double & tolerance);
int   IntersectPoly(POLYGON *p1,POLYGON *p2,int *visible2,POLYGON **result);
double GetInterArea(POLYGON *inP1,POLYGON *inP2,int *edgeVisible,float *uC,float *vC,int *nbV,double **lList);
double GetInterAreaBF(POLYGON *inP1,double u0,double v0,double u1,double v1,float *uC,float *vC);

