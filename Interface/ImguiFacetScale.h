#pragma once
#include "ImguiWindowBase.h"

class ImFacetScale : public ImWindow {
public:
	void Draw();
protected:
	void GetSelectedFacetButtonPress();
	void ScaleFacetButtonPress();
	void CopyFacetButtonPress();
	enum InvariantPoint : short {
		Coords,
		Vertex,
		Facet
	};
	InvariantPoint invPt = Coords;
	enum ScaleFactor : short {
		Uniform,
		Distorted
	};
	ScaleFactor scaleFac = Uniform;
	enum Axis : short { // so arrays corresponding to axies can be accessed by axis names
		X = 0,
		Y = 1,
		Z = 2
	};
	std::string invariantPointInput[3] = { "0","0","0" }, facetIdInput = "0", distortedScaleFactorInput[3] = { "1","1" ,"1" } , uniformScaleNumeratorInput = "1", uniformScaleDenominatorInput = "";
	double invariantPoint[3] = { 0,0,0 }, distortedScaleFactor[3] = { 1,1,1 }, uniformScaleNumerator=1, uniformScaleDenumerator=1;
	size_t facetId = 0;
};