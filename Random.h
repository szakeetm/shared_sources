/*
  File:        Random.h
  Description: Random generator (Mersenne twister)
               Grabbed from NumPy. http://numpy.scipy.org/
  Program:     MolFlow
  Author:      R. KERSEVAN / J-L PONS / M ADY
  Copyright:   E.S.R.F / CERN

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#ifndef _RANDOMH_
#define _RANDOMH_

// Initialise the random generator with the specified seed
extern void   rseed(unsigned long seed);

// Returns a uniform distributed double value in the interval ]0,1[
extern double rnd();

#endif /* _RANDOMH_ */
