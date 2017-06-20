#pragma once
#include "Vector.h"

typedef struct {

  Vector2d  *pts;   // Array of 2D vertex
  size_t     nbPts; // Number of vertex
  double     sign;  // Polygon orientation

} POLYGON;

typedef struct  {

  Vector2d  p;       // Vertex coordinates
  int       mark;    // Cycle detection (0=>not processed, 1=>processed)
  int       isStart; // Possible starting point

  size_t       nbOut;  // Number of outgoing arc
  size_t       nbIn;   // Number of incoming arc
  int       VI[2];  // Tangent point detection, can be -1
  int       VO[2];  // Tangent point detection, can be -1

} POLYVERTEX;

typedef struct {

	size_t i1;  // Node 1 index
	size_t i2;  // Node 2 index
	size_t s;   // Source polygon (tangent point detection)

} POLYARC;

typedef struct {

  size_t         nbNode;  // Number of node
  POLYVERTEX *nodes;   // Nodes
  size_t         nbArc;   // Number of arc
  POLYARC    *arcs;    // Arcs

} POLYGRAPH;

bool   IsConvex(POLYGON *p,size_t idx);
bool   IsInsideTri(Vector2d *p,Vector2d *p1,Vector2d *p2,Vector2d *p3);
bool   ContainsConcave(POLYGON *p,int i1,int i2,int i3);
bool   EmptyTriangle(POLYGON *p,int i1,int i2,int i3,Vector2d *center);
bool   IsInPoly(double u,double v,Vector2d *pts,size_t nbPts);
bool   IsOnPolyEdge(const double & u, const double & v, Vector2d * pts, const size_t & nbPts, const double & tolerance);
bool   IsOnSection(const double & u, const double & v, const double & baseU, const double & baseV, const double & targetU, const double & targetV, const double & tolerance);
int   IntersectPoly(POLYGON *p1,POLYGON *p2,bool *visible2,POLYGON **result);
double GetInterArea(POLYGON *inP1,POLYGON *inP2,bool *edgeVisible,float *uC,float *vC,size_t *nbV,double **lList);
double GetInterAreaBF(POLYGON *inP1,double u0,double v0,double u1,double v1,float *uC,float *vC);

