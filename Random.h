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
#ifndef _RANDOMH_
#define _RANDOMH_

#include <TruncatedGaussian\rtnorm.hpp>

// Initialise the random generator with the specified seed
extern void   rseed(unsigned long seed);
// Returns a uniform distributed double value in the interval ]0,1[
double rnd();
double Gaussian(const double &sigma);
double TruncatedGaussian(gsl_rng *gen, const double &mean, const double &sigma, const double &lowerBound, const double &upperBound);

#endif /* _RANDOMH_ */
