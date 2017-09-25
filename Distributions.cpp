#include <malloc.h>
#include <cstring>
#include <math.h>
#include "Distributions.h"
#include "File.h"
#include "Random.h"
#include "GLApp/MathTools.h"
#include "GLApp\GLTypes.h"
#include "Shared.h"
#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <math.h>
#include <tuple>



std::vector<double> DistributionND::InterpolateY(const double & x, const bool & allowExtrapolate)
{
	return InterpolateVectorY(x, values, isLogLog, allowExtrapolate);
}

double DistributionND::InterpolateX(const double & y, const size_t & elementIndex, const bool & allowExtrapolate)
{
	return InterpolateVectorX(y, values, elementIndex, isLogLog, allowExtrapolate);
}

double Distribution2D::InterpolateY(const double &x, const bool& allowExtrapolate) const {
	return InterpolateXY(x, values, true, isLogLog, allowExtrapolate);
}

double Distribution2D::InterpolateX(const double &y, const bool& allowExtrapolate) const {
	return InterpolateXY(y, values, false, isLogLog, allowExtrapolate);
}

