#include "Buffer_shared.h"

FacetHitBuffer & FacetHitBuffer::operator+=(const FacetHitBuffer & rhs)
{
	this->fluxAbs += rhs.fluxAbs;
	this->nbAbsEquiv += rhs.nbAbsEquiv;
	this->nbDesorbed += rhs.nbDesorbed;
	this->nbHitEquiv += rhs.nbHitEquiv;
	this->nbMCHit += rhs.nbMCHit;
	this->powerAbs += rhs.powerAbs;
	return *this;
}
