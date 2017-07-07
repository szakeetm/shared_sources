#include "Polygon.h"
#include "GLApp/MathTools.h"
#include <math.h>
//Vector.h included in header
#include <stdlib.h>
#include <string.h> //memcpy

bool IsConvex(POLYGON *p,size_t idx) {

  // Check if p.pts[idx] is a convex vertex (calculate the sign of the oriented angle)

  size_t i1 = IDX((int)idx-1,p->nbPts); //idx can be 0
  size_t i2 = IDX(idx  ,p->nbPts);
  size_t i3 = IDX(idx+1,p->nbPts);

  double d = DET22(p->pts[i1].u - p->pts[i2].u,p->pts[i3].u - p->pts[i2].u,
                   p->pts[i1].v - p->pts[i2].v,p->pts[i3].v - p->pts[i2].v);

  return (d*p->sign)>=0.0;

}

bool IsInsideTri(Vector2d *p,Vector2d *p1,Vector2d *p2,Vector2d *p3)
{

  // Check if p is inside the triangle p1 p2 p3
  Vector2d pts[3];
  pts[0] = *p1;
  pts[1] = *p2;
  pts[2] = *p3;
  return IsInPoly(p->u,p->v,pts,3);

}

bool ContainsConcave(POLYGON *p,int i1,int i2,int i3)
{

  // Determine if the specified triangle contains or not a concave point
  size_t _i1 = IDX(i1,p->nbPts);
  size_t _i2 = IDX(i2,p->nbPts);
  size_t _i3 = IDX(i3,p->nbPts);

  Vector2d *p1 = p->pts + _i1;
  Vector2d *p2 = p->pts + _i2;
  Vector2d *p3 = p->pts + _i3;

  int found = 0;
  int i = 0;
  while(!found && i<p->nbPts) {
    if( i!=_i1 && i!=_i2 && i!=_i3 ) { 
      Vector2d *pt = p->pts + i;
      if( IsInsideTri(pt,p1,p2,p3) )
        found = !IsConvex(p,i);
    }
    i++;
  }

  return found;

}

bool EmptyTriangle(POLYGON *p,int i1,int i2,int i3,Vector2d *center)
{

  // Determine if the specified triangle contains or not an other point of the poly
	size_t _i1 = IDX(i1,p->nbPts);
	size_t _i2 = IDX(i2,p->nbPts);
	size_t _i3 = IDX(i3,p->nbPts);

  Vector2d *p1 = p->pts + _i1;
  Vector2d *p2 = p->pts + _i2;
  Vector2d *p3 = p->pts + _i3;

  bool found = false;
  size_t i = 0;
  while(!found && i<p->nbPts) {
    if( i!=_i1 && i!=_i2 && i!=_i3 ) { 
      Vector2d *pt = p->pts + i;
      found = IsInsideTri(pt,p1,p2,p3);
    }
    i++;
  }

  center->u = (p1->u + p2->u + p3->u)/3.0;
  center->v = (p1->v + p2->v + p3->v)/3.0;
  return !found;

}

bool IsInPoly(double u,double v,Vector2d *pts,size_t nbPts)
{

   // 2D polygon "is inside" solving
   // Using the "Jordan curve theorem" (we intersect in v direction here)

   int n_updown,n_found,j;
   double x1,x2,y1,y2,a,minx,maxx;

   n_updown=0;
   n_found=0;

   for (j = 0; j < (int)nbPts-1; j++) {

     x1 = pts[j].u;
     y1 = pts[j].v;
     x2 = pts[j+1].u;
     y2 = pts[j+1].v;

     if( x2>x1 ) { minx=x1;maxx=x2; } 
     else        { minx=x2;maxx=x1; }

     if (u > minx && u <= maxx) {
         a = (y2 - y1) / (x2 - x1);
         if ((a*(u-x1) + y1) < v) {
           n_updown = n_updown + 1;
         } else {
           n_updown = n_updown - 1;
         }
         n_found++;
     }

   }

   // Last point
   x1 = pts[j].u;
   y1 = pts[j].v;
   x2 = pts[0].u;
   y2 = pts[0].v;

   if( x2>x1 ) { minx=x1;maxx=x2; } 
   else        { minx=x2;maxx=x1; }

   if (u > minx && u <= maxx) {
       a = (y2 - y1) / (x2 - x1);
       if ((a*(u-x1) + y1) < v) {
         n_updown = n_updown + 1;
       } else {
         n_updown = n_updown - 1;
       }
       n_found++;
   }

   if (n_updown<0) n_updown=-n_updown;
   return (((n_found/2)&1) ^ ((n_updown/2)&1));

}

bool IsOnPolyEdge(const double & u, const double & v, Vector2d * pts, const size_t & nbPts, const double & tolerance)
{
	bool onEdge = false;
	for (int i = 0;!onEdge && i < nbPts;i++) {
		double x1 = pts[i].u;
		double y1 = pts[i].v;
		double x2 = pts[(i + 1) % nbPts].u;
		double y2 = pts[(i + 1) % nbPts].v;
		onEdge = IsOnSection(u, v, x1, y1, x2, y2, tolerance);
	}
	return onEdge;
}

bool IsOnSection(const double & u, const double & v, const double & baseU, const double & baseV, const double & targetU, const double & targetV, const double & tolerance)
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
	double distance = fabs(distV*u - distU*v + targetU*baseV - targetV*baseU) / sqrt(pow(distV, 2) + pow(distU, 2));
	return distance < tolerance;
}

int IsOnEdge(Vector2d *p1,Vector2d *p2,Vector2d *p)
{

  // Returns 1 wether p lies in [P1P2], 0 otherwise

  double t = Dot(*p2-*p1,*p-*p1);

  double n1 = (*p - *p1).Norme();
  double n2 = (*p2 - *p1).Norme();

  double c = t / ( n1*n2 );

  // Check that p1,p2 and p are aligned
  if( c != 1.0 )  return 0;

  // Check wether p is between p1 and p2
  t = n1 / n2;
  return (t>=0.0 && t<=1.0);

}

int GetNode(POLYGRAPH *g,Vector2d *p)
{

  // Search a node in the polygraph and returns its id (-1 not found)

  int found = 0;
  int i = 0;
  size_t nb = g->nbNode;

  while(i<nb && !found) {
    found = VertexEqual(&(g->nodes[i].p),p);
    if(!found) i++;
  }

  if( found )
    return i;
  else
    return -1;

}

int SearchFirst(POLYGRAPH *g,POLYVERTEX **s)
{

  // Search a not yet processed starting point
  //(Used by IntersectPoly)

  int found = 0;
  int i = 0;

  while(i<g->nbNode && !found) {
    found = (g->nodes[i].mark == 0) && (g->nodes[i].isStart);
    if(!found) i++;
  }

  if(found)
    *s = &(g->nodes[i]);

  return found;

}

size_t AddNode(POLYGRAPH *g,Vector2d *p)
{
  
  // Add a node to the polygraph and returns its id

  int i = GetNode(g,p);

  if(i<0) {
    // New node
	size_t nb = g->nbNode;
    g->nodes[nb].p = *p;
    g->nodes[nb].VI[0] = -1;
    g->nodes[nb].VI[1] = -1;
    g->nodes[nb].VO[0] = -1;
    g->nodes[nb].VO[1] = -1;
    g->nbNode++;
    return nb;    
  }

  return i;

}

size_t AddArc(POLYGRAPH *g,const size_t &i1,const size_t &i2,const size_t &source)
{

  // Add an arc to the polygraph and returns its id

  int found = 0;
  size_t i = 0;
  size_t nb = g->nbArc;

  while(i<nb && !found) {
    found = (g->arcs[i].i1==i1 && g->arcs[i].i2==i2);
    if(!found) i++;
  }

  if(!found) {
    // New arc
    g->arcs[nb].i1 = i1;
    g->arcs[nb].i2 = i2;
    g->arcs[i].s   = source;
    g->nbArc++;
  }

  return i;

}

void CutArc(POLYGRAPH *g, size_t idx, size_t ni)
{

  // Cut the arc idx by inserting ni
  if( g->arcs[idx].i1!=ni && g->arcs[idx].i2!=ni ) {
	  size_t tmp = g->arcs[idx].i2;
    g->arcs[idx].i2 = ni;
    AddArc(g,ni,tmp,1);
  }

}

void InsertEdge(POLYGRAPH *g,Vector2d *p1,Vector2d *p2,const int &a0)
{

  // Insert a polygon edge in the polygraph, a0 = first arc to be checked

  if( VertexEqual(p1,p2) )
    // Does not add null arc
    return;

  // Insert nodes
  size_t n1 = AddNode(g,p1);
  size_t n2 = AddNode(g,p2);

  // Check intersection of the new arc with the arcs of the first polygon.
  int itFound = 0;
  int i = a0;
  while(i<g->nbArc && !itFound) {

    if( g->arcs[i].s==1 ) {

      Vector2d *e1 = &(g->nodes[ g->arcs[i].i1 ].p);
      Vector2d *e2 = &(g->nodes[ g->arcs[i].i2 ].p);
      Vector2d I;

      if( Intersect2D(p1,p2,e1,e2,&I) ) {
		size_t ni = AddNode(g,&I);
        InsertEdge(g,p1,&I,i+1);
        InsertEdge(g,&I,p2,i+1);
        CutArc(g,i,ni);
        itFound = 1;
      }
    
    }
    i++;

  }

  if( !itFound ) AddArc(g,n1,n2,2);

}

void ClearGraph(POLYGRAPH *g)
{
  free(g->arcs);
  free(g->nodes);
}

POLYGON *CopyPoly(POLYGON *p)
{

   POLYGON *polys = (POLYGON *)malloc( sizeof(POLYGON) );
   polys[0].nbPts = p->nbPts;
   polys[0].pts = (Vector2d *)malloc( p->nbPts * sizeof(Vector2d) );
   memcpy(polys[0].pts , p->pts , p->nbPts * sizeof(Vector2d) );
   polys[0].sign = p->sign;

   return polys;

}

void CreateGraph(POLYGRAPH *g,POLYGON *inP1,POLYGON *inP2,bool *visible2)
{

  // Create the polygraph which represent the 2 intersected polygons
  // with their oriented edges.

	size_t MAXEDGE = inP1->nbPts * inP2->nbPts + 1;

  g->nodes = (POLYVERTEX *)malloc( MAXEDGE * sizeof(POLYVERTEX) );
  memset(g->nodes, 0, MAXEDGE * sizeof(POLYVERTEX) );
  g->arcs = (POLYARC *)malloc( MAXEDGE * 4 * sizeof(POLYARC) );
  memset(g->arcs, 0, MAXEDGE * sizeof(POLYARC) );

  // Fill up the graph with the 1st polygon
  g->nbArc = inP1->nbPts;
  g->nbNode = inP1->nbPts;

  for(size_t i=0;i<inP1->nbPts;i++) {
    g->nodes[i].p = inP1->pts[i];
    g->nodes[i].VI[0] = -1;
    g->nodes[i].VI[1] = -1;
    g->nodes[i].VO[0] = -1;
    g->nodes[i].VO[1] = -1;
    g->arcs[i].i1 = i;
    g->arcs[i].i2 = IDX(i+1,inP1->nbPts);
    g->arcs[i].s = 1;
  }

  // Intersect with 2nd polygon
  for(size_t i=0;i<inP2->nbPts;i++)  {
	  size_t i2 = IDX(i+1,inP2->nbPts);
    if( (!visible2 || (visible2 && visible2[i])) ) {
      if( inP2->sign < 0.0 ) {
        InsertEdge(g,inP2->pts+i2,inP2->pts+i,0);
      } else {
        InsertEdge(g,inP2->pts+i,inP2->pts+i2,0);
      }
    }
  }

  // Remove tangent edge
  for(size_t i=0;i<g->nbArc;i++) {
    if( (g->arcs[i].s>0) ) {
		size_t j = i+1;
      bool found = false;
      while(j<g->nbArc && !found) {
        if( (g->arcs[j].s>0) &&
            (g->arcs[j].i1 == g->arcs[i].i2) &&
            (g->arcs[j].i2 == g->arcs[i].i1) )
        {
          g->arcs[i].s = 0;
          g->arcs[j].s = 0;
		  //found=true??
        }
        if(!found) j++;
      }
    }
  }

  // Fill up successor in the polyvertex array to speed up search
  // of next vertices

  for(size_t i=0;i<g->nbArc;i++) {
    if(g->arcs[i].s>0) {
		size_t idxO = g->arcs[i].i1;
		size_t idxI = g->arcs[i].i2;
      if( g->nodes[idxI].nbIn<2 ) {
        g->nodes[idxI].VI[ g->arcs[i].s-1 ]=(int)g->arcs[i].i1;
        g->nodes[idxI].nbIn++;
      }
      if( g->nodes[idxO].nbOut<2 ) {
        g->nodes[idxO].VO[ g->arcs[i].s-1 ]=(int)g->arcs[i].i2;
        g->nodes[idxO].nbOut++;
      }
    }
  }

  // Mark starting points (2 outgoing arcs)

  for(size_t i=0;i<g->nbNode;i++) {
    if( g->nodes[i].nbOut>=2 ) {
      if( g->nodes[i].nbIn>=2 ) {
        
        // Check tangent point
        Vector2d vi1,vi2,vo1,vo2;
        
		// TO DEBUG!!! Causes frequent crashes
		if (g->nodes[i].VI[0] >= 0 && g->nodes[i].VO[0] >= 0 && g->nodes[i].VI[1] >= 0 && g->nodes[i].VO[1] >= 0) {
			vi1 = g->nodes[g->nodes[i].VI[0]].p - g->nodes[i].p;
			vo1 = g->nodes[g->nodes[i].VO[0]].p - g->nodes[i].p;

			vi2 = g->nodes[g->nodes[i].VI[1]].p - g->nodes[i].p;
			vo2 = g->nodes[g->nodes[i].VO[1]].p - g->nodes[i].p;
		}

        double angI  = GetOrientedAngle(&vi1,&vo1);
        double angII = GetOrientedAngle(&vi1,&vi2);
        double angIO = GetOrientedAngle(&vi1,&vo2);

        g->nodes[i].isStart = (angII<angI) || (angIO<angI);

      } else {
        g->nodes[i].isStart = 1;
      }
    }
  }
}

int CheckLoop(POLYGRAPH *g)
{

  // The grapth is assumed to not contains starting point
  // 0 => More than one loop
  // 1 => tangent node/egde detected

  POLYVERTEX *s,*s0;
  s = &(g->nodes[0]);
  s0 = s;
  int nbVisited=0;
  int ok = s->nbOut == 1;

  do {
    if( ok ) {
      if( s->VO[0]>=0 )
        s = &(g->nodes[s->VO[0]]);
      else
        s = &(g->nodes[s->VO[1]]);
      ok = (s->nbOut == 1);
      nbVisited++;
    }
  } while( (s0!=s) && (ok) );

  // !ok                   => only tangent node
  // nbVisited==g->nbNode  => only tangent edge

  //if( !ok || nbVisited==g->nbNode )
  if( nbVisited==g->nbNode )
    return 1;

  return 0;

}

void FreePolys(POLYGON **polys,size_t nbPoly)
{

  POLYGON *p = *polys;
  for(size_t i=0;i<nbPoly;i++) free(p[i].pts);
  free(p);
  *polys=NULL;

}

int IntersectPoly(POLYGON *inP1,POLYGON *inP2,bool *visible2,POLYGON **result)
{

  // Computes the polygon intersection between p1 and p2.
  // Operates on simple polygon only. (Hole connection segments 
  // must be marked non visible in visible2, visible2=NULL => all are visible)
  // Return the number of polygon created (0 on null intersection, -1 on failure)

  POLYGRAPH graph;
  POLYGRAPH *g=&graph;
  size_t MAXEDGE = inP1->nbPts * inP2->nbPts + 1;

  // Create polygraph
  CreateGraph(g,inP1,inP2,visible2);

  // Search a divergent point
  POLYVERTEX *s,*s0;

  if( !SearchFirst(g,&s) ) {

    // Check particular cases

    if( (g->nbNode==inP1->nbPts) && (g->nbNode==inP2->nbPts) ) {
      // P1 and P2 are equal
      ClearGraph(g);
      *result = CopyPoly(inP1);
      return 1;
    }

    if( CheckLoop(g) ) {
      // Only tangent edge/point found => null intersection
      ClearGraph(g);
      *result = NULL;
      return 0;
    }

    ClearGraph(g);
    int i;
	bool insideP1 = false;
	bool insideP2 = false;

    i=0;
    while( i<inP1->nbPts && !insideP2 ) {
      insideP2 = IsInPoly(inP1->pts[i].u,inP1->pts[i].v,inP2->pts,inP2->nbPts);
      i++;
    }
    if( insideP2 ) {
      // P1 is fully inside P2
      *result = CopyPoly(inP1);
      return 1;
    }

    i=0;
    while( i<inP2->nbPts && !insideP1 ) {
      insideP1 = IsInPoly(inP2->pts[i].u,inP2->pts[i].v,inP1->pts,inP1->nbPts);
      i++;
    }
    if( insideP1 ) {
      // P2 is fully inside P1
      *result = CopyPoly(inP2);
      return 1;
    }

    // Null intersection
    *result = NULL;
    return 0;

  }

  // Compute intersection
  POLYGON *polys = (POLYGON *)malloc( sizeof(POLYGON)*256 );
  int nbPoly = 0;
  int eop;
  Vector2d n1,n2;
  double sine;

  do {

    // Starts a new polygon
    polys[nbPoly].pts   = (Vector2d *)malloc(MAXEDGE*sizeof(Vector2d));
    polys[nbPoly].sign  = 1.0;
    polys[nbPoly].nbPts = 0;
    nbPoly++;
    eop = 0;
    s0 = s;

    while( !eop && polys[nbPoly-1].nbPts<MAXEDGE ) {

      // Add point to the current polygon
      polys[nbPoly-1].pts[polys[nbPoly-1].nbPts] = s->p;
      polys[nbPoly-1].nbPts++;
      s->mark = 1;

      // Go to next point
      switch( s->nbOut ) {

        case 1:

          // Next point
          if(s->VO[0]>=0) {
            // On a P1 edge
            s = &(g->nodes[s->VO[0]]);
          } else {
            // On a P2 edge
            s = &(g->nodes[s->VO[1]]);
          }
          break;

        case 2:

          if( s->VO[0]==-1 || s->VO[1]==-1 ) {
            //Failure!!! (tangent edge not marked)
            FreePolys(&polys,nbPoly);
            ClearGraph(g);
            *result = NULL;
            return -1;
          }
            
          // We have to turn left
          n1 = g->nodes[s->VO[0]].p;
          n2 = g->nodes[s->VO[1]].p;

          sine = DET22(n1.u-(s->p.u),n2.u-(s->p.u),
                       n1.v-(s->p.v),n2.v-(s->p.v));

          if( sine<0.0 )
            // Go to n1
            s = &(g->nodes[s->VO[0]]);
          else
            // Go to n2
            s = &(g->nodes[s->VO[1]]);

          break;

        default:
          //Failure!!! (not ended polygon)
          FreePolys(&polys,nbPoly);
          ClearGraph(g);
          *result = NULL;
          return -1;

      }

      // Reach start point, end of polygon
      eop = (s0==s); 

    }

    if( !eop ) {
      //Failure!!! (inner cycle found)
      FreePolys(&polys,nbPoly);
      ClearGraph(g);
      *result = NULL;
      return -1;
    }

  } while( SearchFirst(&graph,&s) );

  ClearGraph(g);
  *result = polys;
  return nbPoly;

}

double GetInterArea(POLYGON *inP1,POLYGON *inP2,bool *edgeVisible,float *uC,float *vC,size_t *nbV,double **lList)
{

  size_t nbPoly;
  double A0;
  POLYGON *polys;
  *nbV = 0;
  *lList = NULL;
  *uC = 0.0;
  *vC = 0.0;

  nbPoly = IntersectPoly(inP1,inP2,edgeVisible,&polys);
  if( nbPoly==(size_t)-1 )
    return 0.0;
  if( nbPoly==0 )
    return 0.0;

  // Count number of pts
  size_t nbE = 0;
  for(size_t i=0;i<nbPoly;i++) nbE += polys[i].nbPts;
  *lList = (double *)malloc(nbE*2*sizeof(double));
  *nbV = nbE;

  // Area
  nbE = 0;
  double sum = 0.0;
  for(size_t i=0;i<nbPoly;i++) {
    double A = 0.0;
    for(size_t j=0;j<polys[i].nbPts;j++) {
      size_t j1 = IDX(j+1,polys[i].nbPts);
      A += polys[i].pts[j].u*polys[i].pts[j1].v - polys[i].pts[j1].u*polys[i].pts[j].v;
      (*lList)[nbE++] = polys[i].pts[j].u;
      (*lList)[nbE++] = polys[i].pts[j].v;
    }
    if( i==0 ) A0 = fabs(0.5 * A);
    sum += fabs(0.5 * A);
  }

  // Centroid (polygon 0)
  double xC = 0.0;
  double yC = 0.0;
  for(int j=0;j<polys[0].nbPts;j++) {
    size_t j1 = IDX(j+1,polys[0].nbPts);
    double d = polys[0].pts[j].u*polys[0].pts[j1].v - polys[0].pts[j1].u*polys[0].pts[j].v;
    xC += ( polys[0].pts[j].u + polys[0].pts[j1].u )*d;
    yC += ( polys[0].pts[j].v + polys[0].pts[j1].v )*d;
  }
  *uC = (float)( xC / (6.0 * A0) );
  *vC = (float)( yC / (6.0 * A0) );

  FreePolys(&polys,nbPoly);

  return sum;
}

double GetInterAreaBF(POLYGON *inP1,double u0,double v0,double u1,double v1,float *uC,float *vC)
{

  // Compute area of the intersection between the (u0,v0,u1,v1) rectangle 
  // and the inP1 polygon using Jordan theorem.
  // Slow but sure.

  int step = 50;
  int nbHit = 0;
  double ui = (u1-u0) / (double)step;
  double vi = (v1-v0) / (double)step;

  for(int i=0;i<step;i++) {
    double uc = u0 + ui*((double)i+0.5);
    for(int j=0;j<step;j++) {
      double vc = v0 + vi*((double)j+0.5);
      if( IsInPoly(uc,vc,inP1->pts,inP1->nbPts) ) {
        nbHit++;
        *uC = (float)uc;
        *vC = (float)vc;
      }
    }
  }

  return (u1-u0)*(v1-v0)*((double)nbHit/(double)(step*step));

}