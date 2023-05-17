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
#include <Helper/MathTools.h>
#include "Buffer_shared.h"

GlobalHitBuffer& GlobalHitBuffer::operator+=(const GlobalHitBuffer& src) {
	this->globalHits += src.globalHits;

	this->distTraveled_total += src.distTraveled_total;
#if defined(MOLFLOW)
	this->distTraveledTotal_fullHitsOnly += src.distTraveledTotal_fullHitsOnly;
#endif
	this->nbLeakTotal += src.nbLeakTotal;

	return *this;
}

#if defined(MOLFLOW)
/**
* \brief += operator, with simple += of underlying structures
* \param rhs reference object on the right hand
* \return address of this (lhs)
*/
FacetHitBuffer& FacetHitBuffer::operator+=(const FacetHitBuffer& rhs) {
	this->nbDesorbed += rhs.nbDesorbed;
	this->nbMCHit += rhs.nbMCHit;
	this->nbHitEquiv += rhs.nbHitEquiv;
	this->nbAbsEquiv += rhs.nbAbsEquiv;
	this->sum_1_per_ort_velocity += rhs.sum_1_per_ort_velocity;
	this->sum_1_per_velocity += rhs.sum_1_per_velocity;
	this->sum_v_ort += rhs.sum_v_ort;
	this->impulse += rhs.impulse;
	this->impulse_square += rhs.impulse_square;
	this->impulse_momentum += rhs.impulse_momentum;
	return *this;
}

FacetHitBuffer operator+(const FacetHitBuffer& lhs, const FacetHitBuffer& rhs) {
	FacetHitBuffer result(lhs);
	result += rhs;
	return result;
}

#endif
#if defined(SYNRAD)
FacetHitBuffer::FacetHitBuffer() {
	this->ResetBuffer();
}

void FacetHitBuffer::ResetBuffer() {
	this->nbMCHit = 0;
	this->nbDesorbed = 0;
	this->nbHitEquiv = 0.0;
	this->nbAbsEquiv = 0.0;
	this->fluxAbs = 0.0;
	this->powerAbs = 0.0;
}

FacetHitBuffer& FacetHitBuffer::operator+=(const FacetHitBuffer& rhs) {
	this->nbMCHit += rhs.nbMCHit;
	this->nbDesorbed += rhs.nbDesorbed;
	this->nbHitEquiv += rhs.nbHitEquiv;
	this->nbAbsEquiv += rhs.nbAbsEquiv;
	this->fluxAbs += rhs.fluxAbs;
	this->powerAbs += rhs.powerAbs;
	return *this;
}
void FacetHistogramBuffer::Resize(const HistogramParams& params) {
	this->nbHitsHistogram.resize(params.recordBounce ? params.GetBounceHistogramSize() : 0);
	this->nbHitsHistogram.shrink_to_fit();
	this->distanceHistogram.resize(params.recordDistance ? params.GetDistanceHistogramSize() : 0);
	this->distanceHistogram.shrink_to_fit();
}

void FacetHistogramBuffer::Reset() {
	ZEROVECTOR(nbHitsHistogram);
	ZEROVECTOR(distanceHistogram);
#if defined(MOLFLOW)
	ZEROVECTOR(timeHistogram);
#endif
}
#endif

