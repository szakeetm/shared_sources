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
#include <cereal/cereal.hpp>
#include <optional> //C++17
#include <cmath>

using FLOAT = float;

class Vector2d;
Vector2d operator+ (const Vector2d &v1, const Vector2d& v2);
Vector2d operator-(const Vector2d &v1, const Vector2d& v2);
Vector2d operator*(const Vector2d &v1, const double& mult);
Vector2d operator*(const double& mult, const Vector2d &v1);
double Dot(const Vector2d &v1, const Vector2d &v2);
std::optional<Vector2d> Intersect2D(const Vector2d &p1, const Vector2d& p2, const Vector2d& p3, const Vector2d& p4);


double GetOrientedAngle(const Vector2d& v1, const Vector2d& v2);
bool VertexEqual(const Vector2d& p1, const Vector2d& p2);

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
        archive(CEREAL_NVP(u), CEREAL_NVP(v));
    }
};

template<typename FLOAT>
class Vector3_t {
public:
    FLOAT x;
    FLOAT y;
    FLOAT z;
    Vector3_t<FLOAT>(){
        this->x = 0.0;
        this->y = 0.0;
        this->z = 0.0;
    };
    Vector3_t<FLOAT>(const Vector3_t<FLOAT>& rhs){
        this->x = rhs.x;
        this->y = rhs.y;
        this->z = rhs.z;
    };
    Vector3_t<FLOAT>(const FLOAT &val){
        this->x = val;
        this->y = val;
        this->z = val;
    };
    Vector3_t<FLOAT>(const FLOAT &x, const FLOAT &y, const FLOAT &z){
        this->x = x;
        this->y = y;
        this->z = z;
    };
    [[nodiscard]] double Norme() const{
        return sqrt(Dot(*this, *this));
    };
    [[nodiscard]] Vector3_t<FLOAT> Normalized() const{
        double factor = 1.0;
        double length = this->Norme();
        if (length > 0.0) factor /= length;
        Vector3_t<FLOAT> result = factor * (*this);
        return result;
    };

    virtual Vector3_t<FLOAT>& operator= (const Vector3_t<FLOAT> &rhs){
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;

        return *this;
    };
    Vector3_t<FLOAT>& operator+=(const Vector3_t<FLOAT> & rhs){
        *this = *this + rhs;
        return *this;
    };
    FLOAT& operator[] (int dim){
        if(dim == 0){
            return x;
        }
        else if(dim == 1){
            return y;
        }
        else {
            return z;
        }
    };
    const FLOAT& operator[] (int dim) const{
        if(dim == 0){
            return x;
        }
        else if(dim == 1){
            return y;
        }
        else {
            return z;
        }
    };

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(x), CEREAL_NVP(y), CEREAL_NVP(z));
    }
};

/*template<>
Vector3_t<FLOAT>& Vector3_t<FLOAT>::operator= (const Vector3_t<FLOAT> &rhs){
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;

    return *this;
};*/

template<typename FLOAT>
Vector3_t<FLOAT> operator+ (const Vector3_t<FLOAT> &v1, const Vector3_t<FLOAT>& v2){
    return Vector3_t<FLOAT>(v1.x + v2.x,
                    v1.y + v2.y,
                    v1.z + v2.z);
};

template<typename FLOAT>
Vector3_t<FLOAT> operator-(const Vector3_t<FLOAT> &v1, const Vector3_t<FLOAT>& v2){
    return Vector3_t<FLOAT>(v1.x - v2.x,
                    v1.y - v2.y,
                    v1.z - v2.z);
};

/*Vector3_t<FLOAT> operator-(const Vector3_t<FLOAT> &v1, const Vector3_t<FLOAT>& v2){
    return Vector3_t<FLOAT>(v1.x - v2.x,
                            v1.y - v2.y,
                            v1.z - v2.z);
};

Vector3_t<FLOAT> operator-(const Vector3_t<FLOAT> &v1, const Vector3_t<FLOAT>& v2){
    return Vector3_t<FLOAT>(v1.x - v2.x,
                            v1.y - v2.y,
                            v1.z - v2.z);
};*/

template<typename FLOAT>
Vector3_t<FLOAT> operator*(const Vector3_t<FLOAT> &v1, const double& mult){
    return Vector3_t<FLOAT>(v1.x * mult,
                    v1.y * mult,
                    v1.z * mult);
};
template<typename FLOAT>
Vector3_t<FLOAT> operator*(const double& mult, const Vector3_t<FLOAT> &v1){
    return v1*mult;
};
template<typename FLOAT>
double Dot(const Vector3_t<FLOAT> &v1, const Vector3_t<FLOAT> &v2){
    return (v1.x)*(v2.x) + (v1.y)*(v2.y) + (v1.z)*(v2.z);
};

template<typename FLOAT>
Vector3_t<FLOAT> CrossProduct(const Vector3_t<FLOAT> &v1, const Vector3_t<FLOAT> &v2){
    return Vector3_t<FLOAT>(
            (v1.y)*(v2.z) - (v1.z)*(v2.y),
            (v1.z)*(v2.x) - (v1.x)*(v2.z),
            (v1.x)*(v2.y) - (v1.y)*(v2.x)
    );
};

template<typename FLOAT>
Vector2d ProjectVertex(const Vector3_t<FLOAT>& v, const Vector3_t<FLOAT>& U, const Vector3_t<FLOAT>& V, const Vector3_t<FLOAT>& origin){
    //Project v on a plane defined by U,V and return the coordinates in base U,V
    Vector3_t<FLOAT> diff = v - origin;
    return Vector2d(Dot(U, diff) / Dot(U,U),Dot(V, diff) / Dot(V,V));
};

template<typename FLOAT>
Vector3_t<FLOAT> Mirror(const Vector3_t<FLOAT>& P, const Vector3_t<FLOAT>& P0, const Vector3_t<FLOAT>& N){
    return P - 2*Dot(P-P0,N)*N;
};
template<typename FLOAT>
Vector3_t<FLOAT> Project(const Vector3_t<FLOAT>& P, const Vector3_t<FLOAT>& P0, const Vector3_t<FLOAT>& N){
    return P - Dot(P - P0, N)*N;
};
template<typename FLOAT>
Vector3_t<FLOAT> Rotate(const Vector3_t<FLOAT>& P, const Vector3_t<FLOAT>& AXIS_P0, const Vector3_t<FLOAT>& AXIS_DIR, const double& theta){
    //theta = theta / 180 * PI; //degree->radians
    Vector3_t dir = AXIS_DIR.Normalized();
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
    costh = std::cos(theta);
    sinth = std::sin(theta);
    precalc1 = -u*x - v*y - w*z;
    return Vector3_t<FLOAT>(
            (a*(v*v + w*w) - u*(b*v + c*w + precalc1))*(1 - costh) + x*costh + (-c*v + b*w - w*y + v*z)*sinth,
            (b*(u*u + w*w) - v*(a*u + c*w + precalc1))*(1 - costh) + y*costh + (c*u - a*w + w*x - u*z)*sinth,
            (c*(u*u + v*v) - w*(a*u + b*v + precalc1))*(1 - costh) + z*costh + (-b*u + a*v - v*x + u*y)*sinth
    );
};

template<typename FLOAT>
Vector3_t<FLOAT> RandomPerpendicularVector(const Vector3_t<FLOAT> &v,const double &length){
    Vector3_t<FLOAT> randomVector=Vector3_t(
            ((double) rand() / (RAND_MAX)) + 1,
            ((double) rand() / (RAND_MAX)) + 1,
            ((double) rand() / (RAND_MAX)) + 1
    );
    Vector3_t<FLOAT> perpendicularVector=CrossProduct(randomVector,v);
    return length*perpendicularVector.Normalized();
}



class InterfaceVertex : public Vector3_t<FLOAT> { //For Interface
public:
    InterfaceVertex() = default;
    InterfaceVertex(const Vector3_t& src){
        x = src.x;
        y = src.y;
        z = src.z;
        selected = false;
    };
    InterfaceVertex& operator=(const Vector3_t& src) override{
        x = src.x;
        y = src.y;
        z = src.z;
        selected = false;

        return *this;
    };
	bool selected=false;
	void SetLocation(const Vector3_t& v);
	/*template<class Archive>
	void serialize(Archive & archive)
	{
		archive(*((Vector3_t<FLOAT>*)this)); //Write base class
		archive(selected);
	}*/
};