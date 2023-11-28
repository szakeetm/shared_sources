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

#include <cstdio>
#include <algorithm> //std::Lower_bound
#include <iterator>
#include <chrono>
#include <cstring> //strdup

bool IsEqual(const double a, const double b, double toleranceRatio) {
	return std::abs(a - b) < std::max(1E-99, std::max(std::abs(a),std::abs(b))*toleranceRatio);
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
	if (table.size() == 1)
		return table[0].second;

	int lowerIndex = lower_index(x, table);
	int tableSize = static_cast<int>(table.size());
	if (lowerIndex == -1) {
		lowerIndex = 0;
		if (!allowExtrapolate)
			return table[lowerIndex].second; //return first element
	}
	else if (lowerIndex == (tableSize - 1)) {
		if (allowExtrapolate) {
			lowerIndex = tableSize - 2;
		}
		else
			return table[lowerIndex].second; //return last element
	}

	double lowerFirst = (logX) ? log10(table[lowerIndex].first) : table[lowerIndex].first;
	double lowerFirstNext = (logX) ? log10(table[lowerIndex + 1].first) : table[lowerIndex + 1].first;
	double xValue = (logX) ? log10(x) : x;

	double delta = lowerFirstNext - lowerFirst;
	double overshoot = xValue - lowerFirst;

	size_t distrYsize = table[0].second.size();
	std::vector<double> result(distrYsize);
	for (size_t e = 0; e < distrYsize; e++)
	{
		double lowerSecond = (logY) ? log10(table[lowerIndex].second[e]) : table[lowerIndex].second[e];
		double lowerSecondNext = (logY) ? log10(table[lowerIndex + 1].second[e]) : table[lowerIndex + 1].second[e];

		result[e] = (logY) ? Pow10(Weigh(lowerSecond, lowerSecondNext, overshoot / delta)) : Weigh(lowerSecond, lowerSecondNext, overshoot / delta);
	}
	return result;
}



//An overload of the function above that accepts C-style arrays as data
int lower_index(double key, const double* data, size_t array_size) {
	const double* it = std::lower_bound(data, data + array_size, key);
	return static_cast<int>(std::distance(data, it))-1; //off by one, see lower_index_universal for explanation
}

int lower_index(const double key, const std::vector<double>& data) {
	auto it = std::lower_bound(data.begin(), data.end(), key);
	return static_cast<int>(std::distance(data.begin(), it)) - 1;
}

int lower_index(const double key, const std::vector<std::pair<double, double>>& data, const bool first) {
	auto it = std::lower_bound(data.begin(), data.end(), key, [&](const std::pair<double, double>& pair, double val) {
		return first ? pair.first < val : pair.second < val;
		});

	return static_cast<int>(std::distance(data.begin(), it))-1;
}

int weighed_lower_index_X(const double  key, const double  weigh, double * A, double * B, const size_t  size)
{
	//interpolates among two lines of a cumulative distribution
	//all elements of line 1 and line 2 must be monotonously increasing (except equal consecutive values)
	//key: lookup value
	//weigh: between 0 and 1 (0: only first distribution, 1: only second distribution)
	//A* and B* : pointers to arrays of 'size' number of CDF values. The first value (not included) is assumed to be 0, the last (not included) is assumed to be 1
	//return value: lower index. If -1, then key is smaller than first element, if 'size-1', then key is larger than last element
	
	if (size == 0) return -1;

	// Handles the case when size is 1
	if (size == 1) {
		double weighed = Weigh(A[0], B[0], weigh);
		return (key < weighed) ? -1 : 0;
	}

	int left = 0;
	int right = static_cast<int>(size - 1);
	int mid;
	double weighed, nextWeighed;

	// Binary search
	while (left <= right) {
		mid = (left + right) / 2;
		weighed = Weigh(A[mid], B[mid], weigh);
		nextWeighed = Weigh(A[mid + 1], B[mid + 1], weigh);

		if (weighed <= key && key < nextWeighed) {
			// key found at index mid
			return mid;
		}
		else if (weighed < key) {
			left = mid + 1;
		}
		else {
			right = mid - 1;
		}
	}

	// Key not found. Check if it is out of the bounds.
	return (mid == 0) ? -1 : static_cast<int>(size - 1);
	
}

double Pow10(const double a) {
	return pow(10,a);
}

std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN) {
	// Ensure all input vectors are normalized.

	// Convert Cartesian (x,y,z) coordinates to polar coordinates in the (nU,nV,N) basis.
	// Note: The facet is parallel to (U,V), and we use its (nU,nV,N) orthonormal basis.
	// Both (nU,nV,N) and (x,y,z) are left-handed.

	// Perform basis change from (x,y,z) to (nU,nV,N).
	// This leverages the fact that (nU,nV,N) is a member of the Special Orthogonal Group (SO(3)).

	// Calculate the dot product of the incident direction with the orthonormal basis.
	double uComponent = Dot(incidentDir, normU);
	double vComponent = Dot(incidentDir, normV);
	double nComponent = Dot(incidentDir, normN);

	// Handle any rounding errors that could cause issues with the 'acos' function.
	Saturate(nComponent, -1.0, 1.0);

	// Convert from (u,v,n) to polar coordinates (inTheta, inPhi).
	double inTheta = acos(nComponent);  // The angle between the incident direction and the normal vector. Ranges from PI/2 to PI.
	double inPhi = atan2(vComponent, uComponent);  // The angle between the projection of the incident direction in the (U,V) plane and the U axis. Ranges from -PI to PI.

	return { inTheta, inPhi };
}

Vector3d
PolarToCartesian(const Vector3d& normU, const Vector3d& normV, const Vector3d& normN, const double theta, const double phi, const bool reverse) {
	// This function converts polar coordinates to Cartesian coordinates in the (normU,normV,normN) basis.
	// Note: (normU,normV,normN) and (x,y,z) are both left-handed.
	// theta is the angle to the normal of the facet normN, phi to normU

	// Convert from polar coordinates (theta, phi) to Cartesian coordinates (u,v,n).
	const double u = sin(theta) * cos(phi);
	const double v = sin(theta) * sin(phi);
	const double n = cos(theta);

	// Get the (normU,normV,normN) orthonormal basis of the facet
	Vector3d U = normU;
	Vector3d V = normV;
	Vector3d N = reverse ? -1.0 * normN : normN;

	// Perform basis change from (normU,normV,normN) to (x,y,z) 
	return u * U + v * V + n * N;
}

double MathHelper::mapRange(double value, double inMin, double inMax, double outMin, double outMax) {
	return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
}