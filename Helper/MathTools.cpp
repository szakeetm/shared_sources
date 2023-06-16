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
#include "MathTools.h"
#include "Random.h"

#include <cmath>
#include <cstdio>
#include <algorithm> //std::Lower_bound
#include <iterator>
#include <chrono>
#include <cstring> //strdup

bool IsEqual(const double a, const double b, double toleranceRatio) {
	return std::abs(a - b) < Max(1E-99, Max(std::abs(a),std::abs(b))*toleranceRatio);
}

size_t IDX(const int i, const size_t nb) {
	//Return circular index restrained within [0..nb[, allows negative index (Python logics: -1=last)
    int ret = i%(int)nb;
    return (ret>=0)?(ret):(ret+nb);
}

size_t IDX(const size_t i, const size_t nb) {
	//Return circular index restrained within [0..nb[
	return i%nb;
}


size_t Next(const int i, const size_t nb, const bool inverseDir) {
	//Returns the next element of a circular index (next of last is first)
	//inverseDir is a helper: when true, returns the previous
	return Next((size_t)i,nb,inverseDir);
}

size_t Next(const size_t i, const size_t nb, const bool inverseDir) {
	//Returns the next element of a circular index (next of last is first)
	//inverseDir is a helper: when true, returns the previous
	if (!inverseDir) {
		size_t next=i+1;
		if (next==nb) next = 0;
		return next;
	} else return Previous(i,nb,false);
}

size_t Previous(const size_t i, const size_t nb, const bool inverseDir) {
	//Returns the previous element of a circular index (previous of first is last)
	//inverseDir is a helper: when true, returns the next
	if (!inverseDir) {
		if (i==0) return nb-1;
		else return i-1;
	} else return Next(i,nb,false);
}

size_t Previous(const int i, const size_t nb, const bool inverseDir) {
	return Previous((size_t)i,nb,inverseDir);
}



size_t GetPower2(size_t n) {
// Return a power of 2 which is greater or equal than n
  if((n & (n-1))==0) {
    // already a power of 2
    return n;
  } else {
    // Get the power of 2 above
    int p = 0;
    while(n!=0) { n = n >> 1; p++; }
    return (size_t)1 << p;
  }

}

double RoundAngle(double a) {
// Return a in [-PI,PI]
  double r=a;
  while(r<-PI) r+=2.0*PI;
  while(r> PI) r-=2.0*PI;
  return r;

}

char* FormatMemory(size_t size) {
	return FormatMemoryLL((long long)size);
}

char* FormatMemoryLL(long long size) {

	static char ret[256];
	const char *suffixStr[] = { "KB", "MB", "GB", "TB", "PB" };
	auto dSize = static_cast<double>(size);
	int suffix = 0;

	while (dSize >= 1024.0 && suffix < 4) {
		dSize /= 1024.0;
		suffix++;
	}

	if (suffix == 0) {
		sprintf(ret, "%u bytes", static_cast<unsigned int>(size));
	}
	else {
		if (fabs(dSize - floor(dSize)) < 1e-3)
			sprintf(ret, "%.0f%s", dSize, suffixStr[suffix - 1]);
		else
			sprintf(ret, "%.2f%s", dSize, suffixStr[suffix - 1]);
	}
	return ret;

}

double Weigh(const double  a, const double  b, const double  weigh)
{
	return a + (b - a)*weigh;
}

std::vector<double> InterpolateVectorY(const double x, const std::vector<std::pair<double, std::vector<double>>>& table, const bool logX, const bool logY, const bool allowExtrapolate) {
	//Same as InterpolateY but returns a vector.
	//Must repeat most of code because C++ doesn't allow runtime evaluated return-type (and only 'bool first' decides what to return)
	if (table.size() == 1) return table[0].second;

	int lowerIndex = my_lower_bound(x, table);

	if (lowerIndex == -1) {
		lowerIndex = 0;
		if (!allowExtrapolate) return table[lowerIndex].second; //return first element
	}
	else if (lowerIndex == (static_cast<int>(table.size()) - 1)) {
		if (allowExtrapolate) {
			lowerIndex = static_cast<int>(table.size()) - 2;
		}
		else return table[lowerIndex].second; //return last element
	}

	double delta = (logX) ? log10(table[lowerIndex + 1].first) - log10(table[lowerIndex].first) : table[lowerIndex + 1].first - table[lowerIndex].first;
	double overshoot = (logX) ? log10(x) - log10(table[lowerIndex].first) : x - table[lowerIndex].first;

	size_t distrYsize = table[0].second.size();
	std::vector<double> result; result.resize(distrYsize);
	for (size_t e = 0; e < distrYsize; e++)
	{
		if (logY) result[e]=Pow10(Weigh(log10(table[lowerIndex].second[e]),log10(table[lowerIndex + 1].second[e]), overshoot / delta)); //log-log interpolation
		else result[e]=Weigh(table[lowerIndex].second[e],table[lowerIndex + 1].second[e],overshoot / delta);
	}
	return result;
}



//An overload of the function above that accepts C-style arrays as data
int lower_bound_c_array(double key, const double* data, size_t array_size) {
	const double* it = std::lower_bound(data, data + array_size, key);

	if (it == data + array_size) {
		// handle this case (key is greater than any element)
		return array_size;
	}
	return std::distance(data, it);
}

inline int my_lower_bound(const double key, const std::vector<double>& data) {
	// Define the getElement function for doubles
	auto getElement = [](const double& value) {
		return value;
	};
	int index = lower_bound_universal(key, data, getElement);
	return index;
}

int my_lower_bound(const double  key, const std::vector<std::pair<double, double>>& data, const bool first)
{
	// Define the getElement function for pairs
	auto getElement = [](const std::pair<double, double>& pair, const bool first) {
		return first?pair.first:pair.second;
	};
	int index = lower_bound_universal(key, data, getElement);
	return index;
}

int weighed_lower_bound_X(const double  key, const double  weigh, double * A, double * B, const size_t  size)
{
	//interpolates among two lines of a cumulative distribution
	//all elements of line 1 and line 2 must be monotonously increasing (except equal consecutive values)
	//key: lookup value
	//weigh: between 0 and 1 (0: only first distribution, 1: only second distribution)
	//A* and B* : pointers to arrays of 'size' number of CDF values. The first value (not included) is assumed to be 0, the last (not included) is assumed to be 1
	//return value: lower index. If -1, then key is smaller than first element, if 'size-1', then key is larger than last element
	
		if (size == 0) return -1;
		if (size == 1) {
			double weighed = Weigh(A[0], B[0], weigh);
			if (key < weighed) return -1;
			else return 0;
		}
		int L = 0;
		int R = (int)(size - 1);
		// continue searching while [imin,imax] is not empty
		int M; double weighed,nextWeighed;
		while (L<=R)
		{
			M = (L + R) / 2;
			weighed = Weigh(A[M], B[M], weigh);
			nextWeighed = Weigh(A[M + 1], B[M + 1], weigh);
			if (weighed <= key && key < nextWeighed) {
				// key found at index M
				return M;
			}
			else if (weighed < key) {
				L = M + 1;
			}
			else  {
				R = M - 1;
			}
		}
		//Not found
		if (M == 0)
			return -1; //key lower than first element
		else
			return (int)size - 1; //key larger than last element
	
}

double Pow10(const double a) {
	return pow(10,a);
}

std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN) {

    //input vectors need to be normalized

    // Get polar coordinates of the incoming particule direction in the (U,V,N) facet space.
    // Note: The facet is parallel to (U,V), we use its (nU,nV,N) orthonormal basis here.
    // (nU,nV,N) and (x,y,z) are both left handed

    // Cartesian(x,y,z) to polar in (nU,nV,N) transformation

    // Basis change (x,y,z) -> (nU,nV,N)
    // We use the fact that (nU,nV,N) belongs to SO(3)
    double u = Dot(incidentDir, normU);
    double v = Dot(incidentDir, normV);
    double n = Dot(incidentDir, normN);
    Saturate(n, -1.0, 1.0); //sometimes rounding errors do occur, 'acos' function would return no value for theta

    // (u,v,n) -> (theta,phi)

    double inTheta = acos(n);              // Angle to normal (PI/2 => PI
    //double rho = sqrt(v*v + u*u);
    //double inPhi = asin(v / rho);     //At this point, -PI/2 < inPhi < PI/2
    //if (u < 0.0) inPhi = PI - inPhi;  // Angle to U
    double inPhi = atan2(v, u); //-PI .. PI, and the angle is 0 when pointing towards u
    return { inTheta, inPhi };
}

Vector3d
PolarToCartesian(const Vector3d &nU, const Vector3d &nV, const Vector3d &nN, const double theta, const double phi,
                 const bool reverse) {

    //returns sHandle->currentParticleTracer.direction

    //Vector3d U, V, N;
    //double u, v, n;

    // Polar in (nU,nV,N) to Cartesian(x,y,z) transformation  ( nU = U/|U| , nV = V/|V| )
    // theta is the angle to the normal of the facet N, phi to U
    // ! See Geometry::InitializeGeometry() for further informations on the (U,V,N) basis !
    // (nU,nV,N) and (x,y,z) are both left handed

    double u = sin(theta)*cos(phi);
    double v = sin(theta)*sin(phi);
    double n = cos(theta);
    //#endif

    // Get the (nU,nV,N) orthonormal basis of the facet
    Vector3d U = nU; // nU
    Vector3d V = nV; // nV
    Vector3d N = nN; // nN
    if (reverse) {
        N = -1.0 * N;
    }
    // Basis change (nU,nV,N) -> (x,y,z)
    return u*U + v*V + n * N;
}