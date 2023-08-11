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
#include "Distributions.h"
#include "Helper/MathTools.h"
#include <vector>


std::vector<double> DistributionND::InterpY(const double  x, const bool  allowExtrapolate)
{
	return InterpolateVectorY(x, values, logXinterp, logYinterp, allowExtrapolate);
}

/*
double DistributionND::InterpolateX(const double  y, const int  elementIndex, const bool  allowExtrapolate)
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

