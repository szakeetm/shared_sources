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
#pragma once

#include <vector>
#include <algorithm> //std::find
#include <string>
#include <cassert>
#include <Vector.h>
#include <cmath>

bool    IsEqual(const double a, const double b, double toleranceRatio=1E-6);
double RoundAngle(double a);
size_t    GetPower2(size_t n);
double Pow10(const double a);
/*
int isinf(double x);
int isnan(double x);
*/

template <typename T1, typename T2, typename T3> void Saturate(T1& x, const T2& min, const T3& max) { if (x<min) x = min; if (x>max) x = max; }
size_t IDX(const int i, const size_t nb);
size_t IDX(const size_t i, const size_t nb);
size_t Next(const int i, const size_t nb, const bool inverseDir=false);
size_t Next(const size_t i, const size_t nb, const bool inverseDir=false);
size_t Previous(const int i, const size_t nb, const bool inverseDir=false);
size_t Previous(const size_t i, const size_t nb, const bool inverseDir=false);

template <typename TYPE> bool IsZero(const TYPE& x) { return std::abs(x)<1E-10; }
template <typename TYPE> TYPE Square(const TYPE& a) { return a*a; }

#define PI 3.14159265358979323846

inline double DET22(double _11, double _12, double _21, double _22) {
	return (_11) * (_22)-(_21) * (_12);
}

inline double DET33(double _11, double _12, double _13,
	double _21, double _22, double _23,
	double _31, double _32, double _33) {
	return (_11) * ((_22) * (_33)-(_32) * (_23))
		+ (_12) * ((_23) * (_31)-(_33) * (_21))
		+ (_13) * ((_21) * (_32)-(_31) * (_22));
}

#define VERY_SMALL 1.0E-30
#define MY_INFINITY 1.e100

char  *FormatMemory(size_t size);
char  *FormatMemoryLL(long long size);

//[[maybe_unused]] double my_erf(double x);
double Weigh(const double a, const double b, const double weigh);

template <typename TYPE> bool Contains(const std::vector<TYPE>& vec, const TYPE& value) {
	return (std::find(vec.begin(), vec.end(), value) != vec.end());
}

template <typename TYPE> size_t FirstIndex(const std::vector<TYPE>& vec, const TYPE& value) {
	return (std::find(vec.begin(), vec.end(), value) - vec.begin());
}

int weighed_lower_index_X(const double key, const double weigh, double* A, double* B, const size_t size);

//Lower index functions
//input: double lookup key, a vector of any type of data, and a getElement function that returns the a double value of T type
//return:	if key larger than last element, returns the size of the vector
//			if key is between discrete data points, it returns the index of the last element that's lower
//			consequently, if smaller than all elements, returns the previous
//warning:	off by one compared to std::lower_bound -> returns the index of an element LOWER than key
int lower_index(const double key, const double* data,const size_t size); //c-style array of double
int lower_index(const double key, const std::vector<double>& data);
int lower_index(const double key, const std::vector<std::pair<double, double>>& data, const bool first); //pairs, search in first or second element
template <typename T> int lower_index(const double  key, const std::vector<T>& data, const bool first)
{
	auto it = std::lower_bound(data.begin(), data.end(), key, [&](const T& obj, double val) {
		return obj.GetElement(first) < val;
		});
	return static_cast<int>(std::distance(data.begin(), it)) - 1;
}
template <typename T> int lower_index(const double  key, const std::vector<std::pair<double, T>>& data)
{
	auto it = std::lower_bound(data.begin(), data.end(), key, [](const std::pair<double, T>& pair, double val) {
		return pair.first < val;
		});

	return static_cast<int>(std::distance(data.begin(), it)) - 1;
}


std::vector<double> InterpolateVectorY(const double x, const std::vector<std::pair<double, std::vector<double>>>& table, const bool logX = false, const bool logY = false, const bool allowExtrapolate = false);
//double InterpolateVectorX(const double y, const std::vector<std::pair<double, std::vector<double>>>& table, const size_t elementIndex, const bool logX=false, const bool logY=false, const bool allowExtrapolate = false);

template <typename T, typename Func>
double InterpolateXY_universal(const double lookupValue, //X or Y depending on searchFirst
	const std::vector<T>& data, //X-Y pairs
	const bool searchFirst, //if true, search in X (and return interpolated Y)
	const bool logX,
	const bool logY,
	const bool allowExtrapolate, //if false, clamped to first/last value
	Func getElement) { //lambda function 
	if (data.empty()) {
		// handle this case appropriately
		return 0.0;
	}

	if (data.size() == 1) {
		return getElement(data[0], !searchFirst);
	}

	//Now we have at least 2 values

	int lowerIndex = lower_index(lookupValue, data, searchFirst); //element below key

	// If lower index is -1, set to 0, or return the first element if not allowing extrapolation.
	if (lowerIndex == -1) {
		if (!allowExtrapolate) {
			return getElement(data[0], !searchFirst); //clamp to first value
		}
		else {
			lowerIndex = 0; //extrapolate based on first 2 datapoints
		}
	}
	// If lower index is at the end, decrease by one, or return the last element if not allowing extrapolation.
	else if (lowerIndex == (static_cast<int>(data.size()) - 1)) {
		if (!allowExtrapolate) {
			return getElement(data[lowerIndex], !searchFirst);
		}
		else {
			lowerIndex = static_cast<int>(data.size()) - 2;
		}
	}

	double valueLowerIndex = getElement(data[lowerIndex], searchFirst);
	double valueLowerIndexNext = getElement(data[lowerIndex + 1], searchFirst);

	// Compute delta and overshoot considering whether to use logarithmic scale or not.
	double delta = (searchFirst ? logX : logY) ? log10(valueLowerIndexNext) - log10(valueLowerIndex)
		: valueLowerIndexNext - valueLowerIndex;

	double overshoot = (searchFirst ? logX : logY) ? log10(lookupValue) - log10(valueLowerIndex)
		: lookupValue - valueLowerIndex;

	// Compute the result considering whether to use logarithmic scale or not.
	if (searchFirst ? logY : logX) {
		return Pow10(Weigh(log10(getElement(data[lowerIndex], !searchFirst)),
			log10(getElement(data[lowerIndex + 1], !searchFirst)),
			overshoot / delta));
	}
	else {
		return Weigh(getElement(data[lowerIndex], !searchFirst),
			getElement(data[lowerIndex + 1], !searchFirst),
			overshoot / delta);
	}
}

//vector of T object consisting of 2 doubles
template <typename T> inline double InterpolateY(const double x, const std::vector<T>& table, const bool logX, const bool logY, const bool allowExtrapolate) {
	auto getElement = [](const T& obj, const bool first) {
		return obj.GetElement(first);
	};
	return InterpolateXY_universal(x, table, true, logX, logY, allowExtrapolate, getElement);
}

//specialization for vector of pair of double,double
template<> inline double InterpolateY<std::pair<double, double>>(const double x, const std::vector<std::pair<double, double>>& table, const bool logX, const bool logY, const bool allowExtrapolate) {
	// Define the getElement function for pairs
	auto getElement = [](const std::pair<double, double>& pair, const bool first) {
		return first ? pair.first : pair.second;
	};
	return InterpolateXY_universal(x, table, true, logX, logY, allowExtrapolate, getElement);
}

//vector of T object consisting of 2 doubles
template <typename T> inline double InterpolateX(const double y, const std::vector<T>& table, const bool logX, const bool logY, const bool allowExtrapolate) {
	auto getElement = [](const T& obj, const bool first) {
		return obj.GetElement(first);
	};
	return InterpolateXY_universal(y, table, false, logX, logY, allowExtrapolate, getElement);
}

//specialization for vector of pair of double,double
template<> inline double InterpolateX<std::pair<double, double>>(const double y, const std::vector<std::pair<double, double>>& table, const bool logX, const bool logY, const bool allowExtrapolate) {
	// Define the getElement function for pairs
	auto getElement = [](const std::pair<double, double>& pair, const bool first) {
		return first ? pair.first : pair.second;
	};
	return InterpolateXY_universal(y, table, false, logX, logY, allowExtrapolate, getElement);
}

std::tuple<double, double> CartesianToPolar(const Vector3d& incidentDir, const Vector3d& normU, const Vector3d& normV, const Vector3d& normN);
Vector3d
PolarToCartesian(const Vector3d &nU, const Vector3d &nV, const Vector3d &nN, const double theta, const double phi,
                 const bool reverse); //sets sHandle->currentParticleTracer.direction

double GeneratePoissonRnd(const double lambda, const double rnd);

//Elementwise addition of two vectors:
#include <algorithm>
#include <functional>
template <class T>
std::vector<T> operator+(const std::vector<T>& lhs, const std::vector<T>& rhs)
{
    assert(lhs.size() == rhs.size());

    std::vector<T> result(lhs.size());

	std::transform(lhs.begin(), lhs.end(), rhs.begin(), result.begin(), std::plus<T>());

	/*
	auto it1 = lhs.begin();
	auto it2 = rhs.begin();
	auto it3 = result.begin();

	while (it1 != lhs.end()) {
		*it3 = *it1 + *it2;
		it1++; it2++; it3++;
	}
	*/

    return result;
}

template <class T>
std::vector<T>& operator+=(std::vector<T>& lhs, const std::vector<T>& rhs)
{
	assert(lhs.size() == rhs.size());

	std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), std::plus<T>());

	/*
	auto it1 = lhs.begin();
	auto it2 = rhs.begin();
	while (it1 != lhs.end()) {
		*it1 += *it2;
		it1++; it2++;
	}
	*/

	return lhs;
}
namespace MathHelper {
	double mapRange(double value, double inMin, double inMax, double outMin, double outMax);
}



#ifdef _WIN32
#define DEBUG_BREAK __debugbreak()
#else
#define DEBUG_BREAK __builtin_trap()
#endif