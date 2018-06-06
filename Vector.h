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
#pragma once

class Vector3d {
public:
	double x;
	double y;
	double z;
	Vector3d();
	Vector3d(const double &x, const double &y, const double &z);
	double Norme() const;
	Vector3d Normalized() const;
	Vector3d& operator+=(const Vector3d & rhs);
	
	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(x, y, z);
	}
};

class Vector2d {
public:
	double u;
	double v;
	Vector2d();
	Vector2d(const double &u, const double &v);
	double Norme() const;
	Vector2d Normalized() const;

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(u,v);
	}
};

class InterfaceVertex : public Vector3d { //For Interface
public:
	InterfaceVertex();
	bool selected;
	void SetLocation(const Vector3d& v);
};

typedef struct {

	Vector3d min;
	Vector3d max;

} AABB;

Vector3d operator+ (const Vector3d &v1, const Vector3d& v2);
Vector3d operator-(const Vector3d &v1, const Vector3d& v2);
Vector3d operator*(const Vector3d &v1, const double& mult);
Vector3d operator*(const double& mult, const Vector3d &v1);
double Dot(const Vector3d &v1, const Vector3d &v2);
Vector3d CrossProduct(const Vector3d &v1, const Vector3d &v2);

Vector2d operator+ (const Vector2d &v1, const Vector2d& v2);
Vector2d operator-(const Vector2d &v1, const Vector2d& v2);
Vector2d operator*(const Vector2d &v1, const double& mult);
Vector2d operator*(const double& mult, const Vector2d &v1);
double Dot(const Vector2d &v1, const Vector2d &v2);

Vector2d ProjectVertex(const Vector3d& v, const Vector3d& U, const Vector3d& V, const Vector3d& origin);
int Intersect2D(Vector2d *p1, Vector2d *p2, Vector2d *p3, Vector2d *p4, Vector2d *I);

Vector3d Mirror(const Vector3d& P, const Vector3d& P0, const Vector3d& N);
Vector3d Project(const Vector3d& P, const Vector3d& P0, const Vector3d& N);
Vector3d Rotate(const Vector3d& P, const Vector3d& AXIS_P0, const Vector3d& AXIS_DIR, const double& theta);

double GetOrientedAngle(Vector2d *v1, Vector2d *v2);
Vector3d RandomPerpendicularVector(const Vector3d &v,const double &length);
int VertexEqual(Vector2d *p1, Vector2d *p2);