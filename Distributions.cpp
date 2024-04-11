
#include "Distributions.h"
#include "Helper/MathTools.h"
#include <vector>


std::vector<double> DistributionND::InterpY(const double  x, const bool  allowExtrapolate)
{
	return InterpolateVectorY(x, values, logXinterp, logYinterp, allowExtrapolate);
}

/*
double DistributionND::InterpolateX(const double  y, const size_t  elementIndex, const bool  allowExtrapolate)
{
	return InterpolateVectorX(y, values, elementIndex, logXinterp, logYinterp, allowExtrapolate);
}
*/

double Distribution2D::InterpY(const double x, const bool allowExtrapolate) const {
	return InterpolateY(x, values, logXinterp, logYinterp,allowExtrapolate); //In MathTools.h
}

double Distribution2D::InterpX(const double y, const bool allowExtrapolate) const {
	return InterpolateX(y, values, logXinterp, logYinterp, allowExtrapolate); //In MathTools.h
}

