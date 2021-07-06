/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#include "Vector.h"
#include "Helper/MathTools.h" //PI
#include <math.h> //sqrt

Vector2d::Vector2d() {}

Vector2d::Vector2d(const double &u, const double &v) {
		this->u = u;
		this->v = v;
}

Vector2d operator+ (const Vector2d &v1, const Vector2d& v2) {
	return Vector2d(v1.u + v2.u,
					v1.v + v2.v);
}

Vector2d operator-(const Vector2d &v1, const Vector2d& v2) {
	return Vector2d(v1.u - v2.u,
					v1.v - v2.v);
}

Vector2d operator*(const Vector2d &v1, const double& mult) {
	return Vector2d(v1.u * mult,
					v1.v * mult);
}

Vector2d operator*(const double& mult, const Vector2d &v1) {
	return v1*mult;
}

double Dot(const Vector2d &v1, const Vector2d &v2) {
	return v1.u*v2.u + v1.v*v2.v;
}

double Vector2d::Norme() const {
	return sqrt(Dot(*this, *this));
}

Vector2d Vector2d::Normalized() const {
	double factor = 1.0;
	double length = this->Norme();
	if (length > 0.0) factor /= length;
	return factor * (*this);
}

bool VertexEqual(const Vector2d& p1, const Vector2d& p2) {
	return IsZero((p1-p2).Norme());
}

std::optional<Vector2d> Intersect2D(const Vector2d &p1, const Vector2d& p2, const Vector2d& p3, const Vector2d& p4) {

  // Computes the intersection between 2 segments
  // Solve P1 + t1P1P2 = P3 + t2P3P4

  Vector2d p12 = p2 - p1;
  Vector2d p13 = p1 - p3;
  Vector2d p34 = p4 - p3;

  double det = DET22(-p12.u,p34.u,
                     -p12.v,p34.v);
  if( IsZero(det) ) return std::nullopt;

  double idet = 1.0 / det;

  double dt1 = DET22(p13.u, p34.u, 
                     p13.v, p34.v);
  double t1  = dt1*idet;
  if( t1<0.0 || t1>1.0 ) return std::nullopt;

  double dt2 = DET22(-p12.u, p13.u, 
                     -p12.v, p13.v);
  double t2  = dt2*idet;
  if( t2<0.0 || t2>1.0 ) return std::nullopt;

  // Check coherence (numerical error)
  Vector2d I1 = p1 + t1 * p12;
  Vector2d I2 = p3 + t2 * p34;
  double r = (I1-I2).Norme();
  if( r>1e-6 ) return std::nullopt;

  return I1;

}

double GetOrientedAngle(const Vector2d& v1,const Vector2d& v2) {

  // Return oriented angle [0,2PI] (clockwise)
  double cs = Dot(v1,v2)/(v1.Norme()*v2.Norme());
  Saturate(cs, -1.0, 1.0);
  double a = acos( cs );
  double s = DET22( v1.u,v2.u,v1.v,v2.v );
  if(s<0.0) a = 2.0*PI - a;
  return 2.0*PI - a;

}

void InterfaceVertex::SetLocation(const Vector3_t<FLOAT>& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}
