#include "Vector.h"
#include "GLApp/MathTools.h" //PI
#include "Random.h" //RandomPerpendicularVector
#include <math.h> //sqrt

Vector2d::Vector2d() {}

Vector2d::Vector2d(const double &u, const double &v) {
		this->u = u;
		this->v = v;
}

Vector3d::Vector3d() {}

Vector3d::Vector3d(const double &x, const double &y,const double &z) {
		this->x = x;
		this->y = y;
		this->z = z;
}

Vector3d operator+ (const Vector3d &v1, const Vector3d& v2) {
	return Vector3d(v1.x + v2.x,
					v1.y + v2.y,
					v1.z + v2.z);
}

Vector3d operator-(const Vector3d &v1, const Vector3d& v2) {
	return Vector3d(v1.x - v2.x,
					v1.y - v2.y,
					v1.z - v2.z);
}

Vector3d operator*(const Vector3d &v1, const double& mult) {
	return Vector3d(v1.x * mult,
					v1.y * mult,
					v1.z * mult);
}

Vector3d operator*(const double& mult, const Vector3d &v1) {
	return v1*mult;
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

Vector3d CrossProduct(const Vector3d &v1, const Vector3d &v2) {
	return Vector3d(
		(v1.y)*(v2.z) - (v1.z)*(v2.y),
		(v1.z)*(v2.x) - (v1.x)*(v2.z),
		(v1.x)*(v2.y) - (v1.y)*(v2.x)
	);
}

double Dot(const Vector3d &v1, const Vector3d &v2) {
	return (v1.x)*(v2.x) + (v1.y)*(v2.y) + (v1.z)*(v2.z);
}


double Dot(const Vector2d &v1, const Vector2d &v2) {
	return v1.u*v2.u + v1.v*v2.v;
}

double Vector3d::Norme() const {
	return sqrt(Dot(*this, *this));
}

Vector3d Vector3d::Normalized() const {
	double factor = 1.0;
	double length = this->Norme();
	if (length > 0.0) factor /= length;
	Vector3d result = factor * (*this);
	return result;
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

Vector3d Mirror(const Vector3d& P, const Vector3d& P0, const Vector3d& N) {
	return P - 2*Dot(P-P0,N)*N;
}

Vector3d Project(const Vector3d& P, const Vector3d& P0, const Vector3d& N) {
	return P - Dot(P - P0, N)*N;
}

Vector3d Rotate(const Vector3d& P, const Vector3d& AXIS_P0, const Vector3d& AXIS_DIR, const double& theta) {
	//theta = theta / 180 * PI; //degree->radians
	Vector3d dir = AXIS_DIR.Normalized();
	double x, y, z, a, b, c, u, v, w, costh, sinth, precalc1;
	x = P.x;
	y = P.y;
	z = P.z;
	a = AXIS_P0.x;
	b = AXIS_P0.y;
	c = AXIS_P0.z;
	u = dir.x;
	v = dir.y;
	w = dir.z;
	costh = cos(theta);
	sinth = sin(theta);
	precalc1 = -u*x - v*y - w*z;
	return Vector3d(
		(a*(v*v + w*w) - u*(b*v + c*w + precalc1))*(1 - costh) + x*costh + (-c*v + b*w - w*y + v*z)*sinth,
		(b*(u*u + w*w) - v*(a*u + c*w + precalc1))*(1 - costh) + y*costh + (c*u - a*w + w*x - u*z)*sinth,
		(c*(u*u + v*v) - w*(a*u + b*v + precalc1))*(1 - costh) + z*costh + (-b*u + a*v - v*x + u*y)*sinth
	);
}

int VertexEqual(Vector2d *p1, Vector2d *p2) {
	return IsZero(p1->u - p2->u) && IsZero(p1->v - p2->v);
}

Vector2d ProjectVertex(const Vector3d& v, const Vector3d& U, const Vector3d& V, const Vector3d& origin){
	//Project v on a plane defined by U,V and return the coordinates in base U,V
	Vector3d diff = v - origin;
	return Vector2d(Dot(U, diff) / Dot(U,U),Dot(V, diff) / Dot(V,V));
}

int Intersect2D(Vector2d *p1,Vector2d *p2,Vector2d *p3,Vector2d *p4,Vector2d *I) {

  // Computes the intersection between 2 segments
  // Return 1 when an intersection is found, 0 otherwise
  // Solve P1 + t1P1P2 = P3 + t2P3P4

  Vector2d p12;
  p12.u = p2->u - p1->u;
  p12.v = p2->v - p1->v;
  Vector2d p34;
  p34.u = p4->u - p3->u;
  p34.v = p4->v - p3->v;

  double det = DET22(-p12.u,p34.u,
                     -p12.v,p34.v);
  if( IsZero(det) ) return 0;

  double idet = 1.0 / det;

  double dt1 = DET22(p1->u-p3->u, p34.u, 
                     p1->v-p3->v, p34.v);
  double t1  = dt1*idet;
  if( t1<0.0 || t1>1.0 ) return 0;

  double dt2 = DET22(-p12.u, p1->u-p3->u, 
                     -p12.v, p1->v-p3->v);
  double t2  = dt2*idet;
  if( t2<0.0 || t2>1.0 ) return 0;

  // Check coherence (numerical error)
  Vector2d I1,I2;
  I1.u = p1->u + t1*p12.u;
  I1.v = p1->v + t1*p12.v;
  I2.u = p3->u + t2*p34.u;
  I2.v = p3->v + t2*p34.v;
  double r = sqrt( (I1.u-I2.u)*(I1.u-I2.u) + (I1.v-I2.v)*(I1.v-I2.v) );
  if( r>1e-6 ) return 0;

  I->u = I1.u;
  I->v = I1.v;
  return 1;

}

double GetOrientedAngle(Vector2d *v1,Vector2d *v2) {

  // Return oriented angle [0,2PI] (clockwise)
  double n = sqrt( (v1->u*v1->u + v1->v*v1->v ) * (v2->u*v2->u + v2->v*v2->v ) );
  double cs = Dot(*v1,*v2)/n;
  if(cs<-1.0) cs=-1.0;
  if(cs>1.0)  cs= 1.0;
  double a = acos( cs );
  double s = DET22( v1->u,v2->u,v1->v,v2->v );
  if(s<0.0) a = 2.0*PI - a;
  return 2.0*PI - a;

}

Vector3d RandomPerpendicularVector(const Vector3d &v,const double &length){
	Vector3d randomVector=Vector3d(rnd(),rnd(),rnd());
	Vector3d perpendicularVector=CrossProduct(randomVector,v);
	return length*perpendicularVector.Normalized();
}

void InterfaceVertex::SetLocation(const Vector3d& v) {
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}