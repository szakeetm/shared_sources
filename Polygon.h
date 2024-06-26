
#pragma once
#include "Vector.h"
#include <tuple>
#include <optional>
#include <vector>
#include <Clipper2Lib/include/clipper2/clipper.h>

class GLAppPolygon { //To distinguish from possible other Polygon classes in the namespace
public:
  std::vector<Vector2d>  pts;   // Array of 2D vertex
  //int sign;  // Polygon orientation
  Vector2d GetCenter();
  double GetArea();
};

class PolyVertex  {
public:
  Vector2d  p;       // Vertex coordinates
  int       mark=0;    // Cycle detection (0=>not processed, 1=>processed)
  int       isStart=0; // Possible starting point

  size_t       nbOut=0;  // Number of outgoing arc
  size_t       nbIn=0;   // Number of incoming arc
  int       VI[2];  // Tangent point detection, can be -1
  int       VO[2];  // Tangent point detection, can be -1
};

class PolyArc {
public:

	size_t i1=0;  // Node 1 index
	size_t i2=0;  // Node 2 index
	size_t s=0;   // Source polygon (tangent point detection)

} ;

/*
class PolyGraph {
public:
  std::vector<PolyVertex> nodes;
  std::vector<PolyArc> arcs;
};
*/

bool   IsConvex(const GLAppPolygon& p,const size_t idx);
bool   ContainsConcave(const GLAppPolygon& p,const int i1,const int i2,const int i3);
//std::tuple<bool,Vector2d>  EmptyTriangle(const GLAppPolygon& p,int i1,int i2,int i3);
bool IsInPoly(const Vector2d& point, const std::vector<Vector2d>& polygon);
bool IsInPoly(const double u, const double v, const std::vector<Vector2d>& polygon);
bool Point_in_triangle(const Vector2d& p, const Vector2d& a, const Vector2d& b, const Vector2d& c); //fast isinpoly for triangle
double sign(const Vector2d& p1, const Vector2d& p2, const Vector2d& p3);
bool   IsOnPolyEdge(const double  u, const double  v, const std::vector<Vector2d>& polyPoints, const double  tolerance);
bool   IsOnSection(const double  u, const double  v, const double  baseU, const double  baseV, const double  targetU, const double  targetV, const double  tolerance);
//std::optional<std::vector<GLAppPolygon>> IntersectPoly(const GLAppPolygon& p1, const GLAppPolygon& p2,const std::vector<bool>& visible2);
//std::tuple<double, Vector2d, std::vector<Vector2d>>  GetInterArea(const GLAppPolygon& inP1,const GLAppPolygon& inP2,const std::vector<bool>& edgeVisible);
std::tuple<double, Vector2d, std::vector<Vector2d>>  GetInterArea_Clipper2Lib(const Clipper2Lib::PathsD& subject, const Clipper2Lib::RectD& rect, const bool isConvex);
std::tuple<double,Vector2d> GetInterAreaBF(const GLAppPolygon& inP1,const Vector2d& p0, const Vector2d& p1);
std::vector<Vector2d> IntersectPolyWithGridline(const std::vector<Vector2d>& poly, const double coord, const bool isX);


