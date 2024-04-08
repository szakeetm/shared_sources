
#pragma once
#include <cereal/cereal.hpp>
#include <optional> //C++17

class Vector3d;
class Vector2d;

Vector3d operator+(const Vector3d &v1, const Vector3d& v2);
Vector3d operator-(const Vector3d &v1, const Vector3d& v2);
Vector3d operator*(const Vector3d &v1, const double mult);
Vector3d operator*(const double mult, const Vector3d &v1);
double Dot(const Vector3d &v1, const Vector3d &v2);
Vector3d CrossProduct(const Vector3d &v1, const Vector3d &v2);

Vector2d operator+ (const Vector2d &v1, const Vector2d& v2);
Vector2d operator-(const Vector2d &v1, const Vector2d& v2);
Vector2d operator*(const Vector2d &v1, const double mult);
Vector2d operator*(const double mult, const Vector2d &v1);
double Dot(const Vector2d &v1, const Vector2d &v2);

Vector2d ProjectVertex(const Vector3d& v, const Vector3d& U, const Vector3d& V, const Vector3d& origin);
std::optional<Vector2d> Intersect2D(const Vector2d &p1, const Vector2d& p2, const Vector2d& p3, const Vector2d& p4);

Vector3d Mirror(const Vector3d& P, const Vector3d& P0, const Vector3d& N);
Vector3d Project(const Vector3d& P, const Vector3d& P0, const Vector3d& N);
Vector3d Rotate(const Vector3d& P, const Vector3d& AXIS_P0, const Vector3d& AXIS_DIR, const double theta);

double GetOrientedAngle(const Vector2d& v1, const Vector2d& v2);
Vector3d RandomPerpendicularVector(const Vector3d &v,const double length);
bool VertexEqual(const Vector2d& p1, const Vector2d& p2);

class Vector3d {
public:
	double x=0.0;
	double y=0.0;
	double z=0.0;
	Vector3d() = default;
	Vector3d(const double x, const double y, const double z);
	double Norme() const;
	Vector3d Normalized() const;
	Vector3d& operator+=(const Vector3d & rhs);
    double& operator[] (int);
	const double& operator[] (int) const;

    template<class Archive>
	void serialize(Archive & archive)
	{
		archive(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z));
	}
};

class Vector2d {
public:
	double u=0.0;
	double v=0.0;
	Vector2d() = default;
	Vector2d(const double u, const double v);
	double Norme() const;
	Vector2d Normalized() const;

	template<class Archive>
	void serialize(Archive & archive)
	{
		archive(CEREAL_NVP(u), CEREAL_NVP(v));
	}
};

class InterfaceVertex : public Vector3d { //For Interface
public:
    InterfaceVertex() = default;
    InterfaceVertex(const Vector3d& src){
        x = src.x;
        y = src.y;
        z = src.z;
        selected = false;
    };
    InterfaceVertex& operator=(const Vector3d& src){
        x = src.x;
        y = src.y;
        z = src.z;
        selected = false;

        return *this;
    };
	bool selected=false;
	void SetLocation(const Vector3d& v);
};