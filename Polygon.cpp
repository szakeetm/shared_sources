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
#include "Polygon.h"
#include "Helper/MathTools.h"
#include <math.h>
#include <algorithm> //min max

bool IsConvex(const GLAppPolygon &p,const int idx) {

  // Check if p.pts[idx] is a convex vertex (calculate the sign of the oriented angle)

  int i1 = Previous(idx,p.pts.size());
  int i2 = IDX(idx, p.pts.size());
  int i3 = Next(idx, p.pts.size());

  double d = DET22(p.pts[i1].u - p.pts[i2].u,p.pts[i3].u - p.pts[i2].u,
                   p.pts[i1].v - p.pts[i2].v,p.pts[i3].v - p.pts[i2].v);

  //return (d*p.sign)>=0.0;
  return d <= 0.0;
}

bool ContainsConcave(const GLAppPolygon &p,const int i1,const int i2,const int i3)
{

  // Determine if the specified triangle contains or not a concave point
  int _i1 = IDX(i1, p.pts.size());
  int _i2 = IDX(i2, p.pts.size());
  int _i3 = IDX(i3, p.pts.size());

  const Vector2d& p1 = p.pts[_i1];
  const Vector2d& p2 = p.pts[_i2];
  const Vector2d& p3 = p.pts[_i3];

  int found = 0;
  int i = 0;
  while(!found && i<p.pts.size()) {
    if( i!=_i1 && i!=_i2 && i!=_i3 ) {
	  if (Point_in_triangle(p.pts[i], p1,p2,p3 ))
        found = !IsConvex(p,i);
    }
    i++;
  }

  return found;

}
/*
std::tuple<bool,Vector2d> EmptyTriangle(const GLAppPolygon& p,int i1,int i2,int i3)
{

  // Determine if the specified triangle contains or not an other point of the poly
	int _i1 = IDX(i1, p.pts.size());
	int _i2 = IDX(i2, p.pts.size());
	int _i3 = IDX(i3, p.pts.size());

    const Vector2d& p1 = p.pts[_i1];
    const Vector2d& p2 = p.pts[_i2];
    const Vector2d& p3 = p.pts[_i3];

  bool found = false;
  int i = 0;
  while(!found && i<p.pts.size()) {
    if( i!=_i1 && i!=_i2 && i!=_i3 ) { 
	  found = Point_in_triangle(p.pts[i], p1,p2,p3 );
    }
    i++;
  }

  return { !found,(1.0 / 3.0)*(p1 + p2 + p3) };

}
*/
bool IsOnPolyEdge(const double  u, const double  v, const std::vector<Vector2d>& polyPoints, const double  tolerance)
{
	bool onEdge = false;
	for (int i = 0;!onEdge && i < polyPoints.size();i++) {
		const double& x1 = polyPoints[i].u;
		const double& y1 = polyPoints[i].v;
		const double& x2 = polyPoints[(i + 1) % polyPoints.size()].u;
		const double& y2 = polyPoints[(i + 1) % polyPoints.size()].v;
		onEdge = IsOnSection(u, v, x1, y1, x2, y2, tolerance);
	}
	return onEdge;
}

bool IsOnSection(const double  u, const double  v, const double  baseU, const double  baseV, const double  targetU, const double  targetV, const double  tolerance)
{
	//Notation from https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
	//u=x0
	//v=y0
	//baseU=x1
	//baseV=y1
	//targetU=x2
	//targetV=y2
	double distU = targetU - baseU;
	double distV = targetV - baseV;
	double distance = std::fabs(distV*u - distU*v + targetU*baseV - targetV*baseU) / sqrt(pow(distV, 2) + pow(distU, 2));
	return distance < tolerance;
}

bool IsOnEdge(const Vector2d& p1,const Vector2d& p2,const Vector2d& p)
{

  // Returns 1 wether p lies in [P1P2], 0 otherwise

  double t = Dot(p2-p1,p-p1);

  double n1 = (p - p1).Norme();
  double n2 = (p2 - p1).Norme();

  double c = t / ( n1*n2 );

  // Check that p1,p2 and p are aligned
  if( c != 1.0 )  return 0;

  // Check wether p is between p1 and p2
  t = n1 / n2;
  return (t>=0.0 && t<=1.0);

}

/*
std::optional<int> GetNode(const PolyGraph& g, const Vector2d& p)
{

  // Search a node in the polygraph and returns its id (-1 not found)

  int found = 0;
  int i = 0;
  int nb = g.nodes.size();

  while(i<nb && !found) {
    found = VertexEqual(g.nodes[i].p,p);
    if(!found) i++;
  }

  if( found )
    return i;
  else
    return std::nullopt;

}

int SearchFirst(const PolyGraph& g)
{

  // Search a not yet processed starting point
  //(Used by IntersectPoly)

  for (int i = 0; i < g.nodes.size(); i++) {
	const PolyVertex& node = g.nodes[i];
	if (node.mark == 0 && node.isStart) return i;
  }
  //Not found
  return -1;
}

int AddNode(PolyGraph& g,const Vector2d& p)
{
  
  // Add a node to the polygraph and returns its id

	if (auto i = GetNode(g, p)) return *i;

	else {
		// New node
		PolyVertex newNode;
		newNode.p = p;
		newNode.VI[0] =
		newNode.VI[1] =
		newNode.VO[0] =
		newNode.VO[1] = -1;
		g.nodes.push_back(newNode);
		return g.nodes.size()-1;    
	 }

}

int AddArc(PolyGraph& g,const int i1,const int i2,const int source)
{

  // Add an arc to the polygraph and returns its id

	for (int i = 0; i < g.arcs.size();i++) {
		if (g.arcs[i].i1==i1 && g.arcs[i].i2==i2) return i;
	  }

	// Not found, new arc
	PolyArc newArc;
	newArc.i1 = i1;
	newArc.i2 = i2;
	newArc.s = source;
	g.arcs.push_back(newArc);
	return g.arcs.size() - 1;

}

void CutArc(PolyGraph& g, int idx, int ni)
{

  // Cut the arc idx by inserting ni
  if( g.arcs[idx].i1!=ni && g.arcs[idx].i2!=ni ) {
	  int tmp = g.arcs[idx].i2;
    g.arcs[idx].i2 = ni;
    AddArc(g,ni,tmp,1);
  }

}


void InsertEdge(PolyGraph& g,const Vector2d& p1,const Vector2d& p2,const int a0)
{

  // Insert a polygon edge in the polygraph, a0 = first arc to be checked

  if( VertexEqual(p1,p2) )
    // Does not add null arc
    return;

  // Insert nodes
  int n1 = AddNode(g,p1);
  int n2 = AddNode(g,p2);

  // Check intersection of the new arc with the arcs of the first polygon.
  bool itFound = false;
  int i = a0;
  while(i<g.arcs.size() && !itFound) {

    if( g.arcs[i].s==1 ) {

      const Vector2d& e1 = g.nodes[ g.arcs[i].i1 ].p;
	  const Vector2d& e2 = g.nodes[ g.arcs[i].i2 ].p;
      Vector2d I;

      if( auto intersectPoint = Intersect2D(p1,p2,e1,e2) ) {
		int ni = AddNode(g,*intersectPoint);
        InsertEdge(g,p1, *intersectPoint,i+1);
        InsertEdge(g, *intersectPoint,p2,i+1);
        CutArc(g,i,ni);
        itFound = 1;
      }
    
    }
    i++;

  }

  if( !itFound ) AddArc(g,n1,n2,2);

}
*/

/*
PolyGraph CreateGraph(const GLAppPolygon& inP1, const GLAppPolygon& inP2,const std::vector<bool>& visible2)
{

  // Create the polygraph which represent the 2 intersected polygons
  // with their oriented edges.

	int MAXEDGE = inP1.pts.size() * inP2.pts.size() + 1;

	PolyGraph g; //result
	g.nodes.resize(inP1.pts.size());
    g.arcs.resize(inP1.pts.size());
	
  // Fill up the graph with the 1st polygon

  for(int i=0;i<inP1.pts.size();i++) {
    g.nodes[i].p = inP1.pts[i];
    g.nodes[i].VI[0] = -1;
    g.nodes[i].VI[1] = -1;
    g.nodes[i].VO[0] = -1;
    g.nodes[i].VO[1] = -1;
    g.arcs[i].i1 = i;
    g.arcs[i].i2 = Next(i,inP1.pts.size());
    g.arcs[i].s = 1;
  }

  // Intersect with 2nd polygon
  for(int i=0;i<inP2.pts.size();i++)  {
	  int i2 = Next(i,inP2.pts.size());
    if( visible2[i] ) {
      //if( inP2.sign < 0 ) {
      //  InsertEdge(g,inP2.pts[i2],inP2.pts[i],0);
      //} else {
        InsertEdge(g,inP2.pts[i],inP2.pts[i2],0);
      //}
    }
  }

  // Remove tangent edge
  for(int i=0;i<g.arcs.size();i++) {
    if( (g.arcs[i].s>0) ) {
		int j = i+1;
      bool found = false;
      while(j<g.arcs.size() && !found) {
        if( (g.arcs[j].s>0) &&
            (g.arcs[j].i1 == g.arcs[i].i2) &&
            (g.arcs[j].i2 == g.arcs[i].i1) )
        {
          g.arcs[i].s = 0;
          g.arcs[j].s = 0;
		  //found=true??
        }
        if(!found) j++;
      }
    }
  }

  // Fill up successor in the polyvertex array to speed up search
  // of next vertices

  for(int i=0;i<g.arcs.size();i++) {
    if(g.arcs[i].s>0) {
		int idxO = g.arcs[i].i1;
		int idxI = g.arcs[i].i2;
      if( g.nodes[idxI].nbIn<2 ) {
        g.nodes[idxI].VI[ g.arcs[i].s-1 ]=(int)g.arcs[i].i1;
        g.nodes[idxI].nbIn++;
      }
      if( g.nodes[idxO].nbOut<2 ) {
        g.nodes[idxO].VO[ g.arcs[i].s-1 ]=(int)g.arcs[i].i2;
        g.nodes[idxO].nbOut++;
      }
    }
  }

  // Mark starting points (2 outgoing arcs)

  for(int i=0;i<g.nodes.size();i++) {
    if( g.nodes[i].nbOut>=2 ) {
      if( g.nodes[i].nbIn>=2 ) {
        
        // Check tangent point
        Vector2d vi1,vi2,vo1,vo2;
        
		// TO DEBUG!!! Causes frequent crashes
		if (g.nodes[i].VI[0] >= 0 && g.nodes[i].VO[0] >= 0 && g.nodes[i].VI[1] >= 0 && g.nodes[i].VO[1] >= 0) {
			vi1 = g.nodes[g.nodes[i].VI[0]].p - g.nodes[i].p;
			vo1 = g.nodes[g.nodes[i].VO[0]].p - g.nodes[i].p;

			vi2 = g.nodes[g.nodes[i].VI[1]].p - g.nodes[i].p;
			vo2 = g.nodes[g.nodes[i].VO[1]].p - g.nodes[i].p;
		}

        double angI  = GetOrientedAngle(vi1,vo1);
        double angII = GetOrientedAngle(vi1,vi2);
        double angIO = GetOrientedAngle(vi1,vo2);

        g.nodes[i].isStart = (angII<angI) || (angIO<angI);

      } else {
        g.nodes[i].isStart = 1;
      }
    }
  }
  return g;
}
*/

/*
bool CheckLoop(const PolyGraph& g)
{

  // The grapth is assumed to not contains starting point
  // 0 => More than one loop
  // 1 => tangent node/egde detected

  const PolyVertex* s = &g.nodes[0];
  const PolyVertex* s0 = s;
  int nbVisited=0;
  bool ok = s->nbOut == 1;

  do {
    if( ok ) {
      if( s->VO[0]>=0 )
        s = &g.nodes[s->VO[0]];
      else
        s = &g.nodes[s->VO[1]];
      ok = (s->nbOut == 1);
      nbVisited++;
    }
  } while( (s0 != s) && (ok) );

  // !ok                   => only tangent node
  // nbVisited==g->nbNode  => only tangent edge

    return nbVisited==g.nodes.size();

}
*/

/*
std::optional<std::vector<GLAppPolygon>> IntersectPoly(const GLAppPolygon& inP1, const GLAppPolygon& inP2, const std::vector<bool>& visible2)
{

	// Computes the polygon intersection between p1 and p2.
	// Operates on simple polygon only. (Hole connection segments 
	// must be marked non visible in visible2)
	// Return the number of polygon created (0 on null intersection, -1 on failure)

	int MAXEDGE = inP1.pts.size() * inP2.pts.size() + 1;

	// Create polygraph
	PolyGraph g = CreateGraph(inP1, inP2, visible2);

	// Search a divergent point


	int vertexId = SearchFirst(g);

	if (vertexId == -1) { //Not found

		// Check particular cases

		if ((g.nodes.size() == inP1.pts.size()) && (g.nodes.size() == inP2.pts.size())) {
			// P1 and P2 are equal
			auto resultVector = { inP1 };
			return resultVector;
		}

		if (CheckLoop(g)) {
			// Only tangent edge/point found => null intersection
			return std::nullopt;
		}

		int i;
		bool insideP1 = false;
		bool insideP2 = false;

		std::vector<Vector2d> p1Pts = inP1.pts;
		std::vector<Vector2d> p2Pts = inP2.pts;

		i = 0;
		while (i < inP1.pts.size() && !insideP2) {
			insideP2 = IsInPoly(inP1.pts[i], p2Pts);
			i++;
		}
		if (insideP2) {
			// P1 is fully inside P2
			auto resultVector = { inP1 };
			return resultVector;
		}

		i = 0;
		while (i < inP2.pts.size() && !insideP1) {
			insideP1 = IsInPoly(inP2.pts[i], p1Pts);
			i++;
		}
		if (insideP1) {
			// P2 is fully inside P1
			auto resultVector = { inP2 };
			return resultVector;
		}

		return std::nullopt;

	}

	// Compute intersection
	int eop;
	Vector2d n1, n2;
	double sine;
	std::vector<GLAppPolygon> polys;
	do {

		// Starts a new polygon
		GLAppPolygon newPoly;
		//newPoly.sign = 1;
		polys.push_back(newPoly);

		eop = 0;
		PolyVertex* s = &g.nodes[vertexId]; //Not a reference, can change
		PolyVertex* s0 = &g.nodes[vertexId];

		while (!eop && polys.back().pts.size() < MAXEDGE) {

			// Add point to the current polygon
			polys.back().pts.push_back(s->p);
			s->mark = 1;

			// Go to next point
			switch (s->nbOut) {

			case 1:

				// Next point
				if (s->VO[0] >= 0) {
					// On a P1 edge
					s = &g.nodes[s->VO[0]];
				}
				else {
					// On a P2 edge
					s = &g.nodes[s->VO[1]];
				}
				break;

			case 2:

				if (s->VO[0] == -1 || s->VO[1] == -1) {
					//Failure!!! (tangent edge not marked)
					return std::nullopt;
				}

				// We have to turn left
				n1 = g.nodes[s->VO[0]].p;
				n2 = g.nodes[s->VO[1]].p;

				sine = DET22(n1.u - (s->p.u), n2.u - (s->p.u),
					n1.v - (s->p.v), n2.v - (s->p.v));

				if (sine < 0.0)
					// Go to n1
					s = &g.nodes[s->VO[0]];
				else
					// Go to n2
					s = &g.nodes[s->VO[1]];

				break;

			default:
				//Failure!!! (not ended polygon)
				return std::nullopt;

			}

			// Reach start point, end of polygon
			eop = (s0 == s);

		}

		if (!eop) {
			//Failure!!! (inner cycle found)
			return std::nullopt;
		}

	} while ((vertexId = SearchFirst(g)) != -1);

	return polys;

}
*/

std::tuple<double, Vector2d, std::vector<Vector2d>> GetInterArea_Clipper2Lib(const Clipper2Lib::PathsD& subjects, const Clipper2Lib::RectD& rect, const bool isConvex) {

	//Clipper2Lib::ClipperD c(8);
	//c.AddSubject(subjects);
	//c.AddClip(clips);
	//Clipper2Lib::PolyTreeD solution;
	//c.Execute(Clipper2Lib::ClipType::Intersection, Clipper2Lib::FillRule::NonZero, solution);
	auto solRect = Clipper2Lib::ExecuteRectClip(rect, subjects,isConvex,8);


	if (solRect.size() == 0) {
		return { 0.0,Vector2d(0.0,0.0),{} };
	}

	Clipper2Lib::PointD centerP(0.0, 0.0);
	auto pts = solRect[0];
	for (int j = 0; j < pts.size(); j++) {
		int j1 = Next(j, pts.size());
		double d = pts[j].x * pts[j1].y - pts[j1].x * pts[j].y;
		centerP = centerP + (pts[j] + pts[j1]) * d;
	}
	Vector2d center(centerP.x, centerP.y); //PointD to Vector2d

	// Count number of pts
	int nbV = 0;
	for (int i = 0; i < solRect.size(); i++) {
		const auto& pts = solRect[i];
		nbV += pts.size();
	}
	std::vector<Vector2d> pointList(nbV);
	
	//Points to list
	int nbE = 0;
	for (int i = 0; i < solRect.size(); i++) {
		const auto& pts = solRect[i];
		for (const auto& p:pts) {
			pointList[nbE++] = Vector2d(p.x,p.y);
		}
	}


	//Area
	double sum = 0.0;
	double A0 = 0.0; //first polygon area
	for (int i = 0; i < solRect.size(); i++) { //loop through outer polygons
		const auto& cPts = solRect[i];
		double A = 0.0; //Current polygon area
		A += Clipper2Lib::Area(cPts);

		/*
		for (int h = 0; h < child->Count(); h++) { //deduct holes
			const auto& hole = child->Child(h);
			const auto& hPts = hole->Polygon();
			A -= Clipper2Lib::Area(hPts);
		}
		*/

		if (i == 0) A0 = std::fabs(A);
		sum += std::fabs(A);

	}

	return { sum,(1.0 / (6.0 * A0)) * center, pointList };
}

/*
std::tuple<double, Vector2d, std::vector<Vector2d>> GetInterArea(const GLAppPolygon& inP1, const GLAppPolygon& inP2, const std::vector<bool>& edgeVisible)
{

	auto polys = IntersectPoly(inP1, inP2, edgeVisible);
	if (!(polys)) return { 0.0,Vector2d(0.0,0.0),{} };

	// Count number of pts
	int nbV = 0;
	for (const auto& p : *polys)
		nbV += p.pts.size();
	std::vector<Vector2d> pointList(nbV);

	//Points to list
	int nbE = 0;
	for (int i = 0; i < (*polys).size(); i++) {
		const auto& polyPts = (*polys)[i].pts;
		for (int j = 0; j < polyPts.size(); j++) {
			pointList[nbE++] = polyPts[j];
		}
	}

	// Area
	double sum = 0.0;
	double A0; //first polygon area
	for (int i = 0; i < (*polys).size(); i++) {
		double A = (*polys)[i].GetArea();
		if (i == 0) A0 = std::fabs(0.5 * A);
		sum += std::fabs(0.5 * A);
	}

	Vector2d center = (*polys)[0].GetCenter();

	return { sum,(1.0 / (6.0 * A0)) * center, pointList };
}
*/

double GLAppPolygon::GetArea() {
	double A = 0.0;
	for (int j = 0; j < pts.size(); j++) {
		int j1 = Next(j, pts.size());
		A += (pts[j].u * pts[j1].v - pts[j1].u * pts[j].v);
	}
	return A;
}

std::tuple<double, Vector2d> GetInterAreaBF(const GLAppPolygon& inP1, const Vector2d& p0, const Vector2d& p1)
{

	// Compute area of the intersection between the (u0,v0,u1,v1) rectangle 
	// and the inP1 polygon using Jordan theorem.
	// Slow but sure, scan by 50x50 matrix and return center

	int step = 50;
	int nbTestHit = 0;
	Vector2d center;
	double ui = (p1.u - p0.u) / (double)step;
	double vi = (p1.v - p0.v) / (double)step;

	for (int i = 0; i < step; i++) {
		double uc = p0.u + ui * ((double)i + 0.5);
		for (int j = 0; j < step; j++) {
			double vc = p0.v + vi * ((double)j + 0.5);
			Vector2d testPoint(uc, vc);
			if (IsInPoly(testPoint, inP1.pts)) {
				nbTestHit++;
				center = testPoint;
			}
		}
	}

	return { (p1.u - p0.u) * (p1.v - p0.v) * ((double)nbTestHit / (double)(step * step)) , center };

}

std::vector<Vector2d> IntersectPolyWithGridline(const std::vector<Vector2d>& poly, const double coord, const bool isX)
{
    //returns list of u,v intersection points of a horizontal or vertical gridline whose distance is 'coord' from 0,0
    //returns empty vector if no intersection, two points for convex facets, possibly more for concave/holed

    std::vector<Vector2d> result;

    for (int i = 0; i < poly.size(); i++) {
        const Vector2d& q1 = poly[i];
        const Vector2d& q2 = poly[(i + 1) % poly.size()];
        if (isX) { //cut by vertical gridline
            if (q1.u == q2.u) {// ignore vertical edges
                continue;
            }
            else if ((q1.u<coord) != (q2.u<coord)) {
                Vector2d diff = q2 - q1;
                double diffPortion = (coord - q1.u) / diff.u;
                Vector2d intersection = q1 + diffPortion * diff;
                result.push_back(intersection);
            }
        }
        else { //cut by horizontal line
            if (q1.v == q2.v) {// ignore horizontal edges
                continue;
            }
            else if ((q1.v < coord) != (q2.v < coord)) {
                Vector2d diff = q2 - q1;
                double diffPortion = (coord - q1.v) / diff.v;
                Vector2d intersection = q1 + diffPortion * diff;
                result.push_back(intersection);
            }
        }
    }

    //Sort
    if (isX) {
        std::sort(std::begin(result), std::end(result),
            [](const auto& lhs, const auto& rhs) {
                return lhs.v < rhs.v;
            });
    }
    else
    {
        std::sort(std::begin(result), std::end(result),
            [](const auto& lhs, const auto& rhs) {
                return lhs.u < rhs.u;
            });
    }

    return result;
}

bool IsInPoly(const Vector2d& point, const std::vector<Vector2d>& polygon) {
    return IsInPoly(point.u, point.v, polygon);
}

//Performance critical! 15% of ray-tracing CPU usage
bool IsInPoly(const double u, const double v, const std::vector<Vector2d>& polygon) {
    // Fast method to check if a point is inside a polygon or not.
    // Works with convex and concave polys, orientation independent
    int n_updown = 0;
    int n_found = 0;
    int n = (int)polygon.size();

    for (int j = 0; j < n; ++j) {
        const Vector2d& p1 = polygon[j];
        const Vector2d& p2 = polygon[Next(j,n)];

        if (u < p1.u != u < p2.u) {
            double slope = (p2.v - p1.v) / (p2.u - p1.u);
            if ((slope * u - v) < (slope * p1.u - p1.v)) {
                n_updown++;
            }
            else {
                n_updown--;
            }
            n_found++;
        }
    }

    return !(n_found & 2u) ^ !(n_updown & 2u);
    
}

// Returns true if the point p is inside the triangle abc, false otherwise
bool Point_in_triangle(const Vector2d& p, const Vector2d& a, const Vector2d& b, const Vector2d& c) {
	double d1 = sign(p, a, b);
	double d2 = sign(p, b, c);
	double d3 = sign(p, c, a);
	bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
	return !(has_neg && has_pos);
}

// Helper function to compute the sign of the cross product of vectors p1->p2 and p1->p3
double sign(const Vector2d& p1, const Vector2d& p2, const Vector2d& p3) {
	return (p1.u - p3.u) * (p2.v - p3.v) - (p2.u - p3.u) * (p1.v - p3.v);
}

Vector2d GLAppPolygon::GetCenter()
{
	Vector2d center(0.0, 0.0);
	// Centroid (polygon 0)
	for (int j = 0; j < pts.size(); j++) {
		int j1 = Next(j, pts.size());
		double d = pts[j].u * pts[j1].v - pts[j1].u * pts[j].v;
		center = center + (pts[j] + pts[j1]) * d;
	}
	return center;
}
