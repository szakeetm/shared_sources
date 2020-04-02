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
#include "Buffer_shared.h"

#ifdef SYNRAD
FacetHitBuffer::FacetHitBuffer(){
    this->ResetBuffer();
}

void FacetHitBuffer::ResetBuffer(){
    this->hit.nbMCHit = 0;
    this->hit.nbDesorbed = 0;
    this->hit.nbHitEquiv = 0.0;
    this->hit.nbAbsEquiv = 0.0;
    this->hit.fluxAbs = 0.0;
    this->hit.powerAbs = 0.0;
}

FacetHitBuffer & FacetHitBuffer::operator+=(const FacetHitBuffer & rhs){
    this->hit.nbMCHit += rhs.hit.nbMCHit;
    this->hit.nbDesorbed += rhs.hit.nbDesorbed;
    this->hit.nbHitEquiv += rhs.hit.nbHitEquiv;
    this->hit.nbAbsEquiv += rhs.hit.nbAbsEquiv;
    this->hit.fluxAbs += rhs.hit.fluxAbs;
	this->hit.powerAbs += rhs.hit.powerAbs;
	return *this;
}
#endif