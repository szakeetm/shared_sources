#pragma once
#include "ImguiWindowBase.h"

class ImFacetExtrude : public ImWindow {
public:
	void Draw();
protected:
	enum Mode : int {
		none,
		facetNormal,
		facetAntinormal,
		directionVector,
		curveNormal,
		curveAntinormal
	};
	Mode mode = none;
	std::string facetLengthInput, pathDXInput, pathDYInput, pathDZInput, curveX0Input, curveY0Input, curveZ0Input,
		curvedXInput, curvedYInput, curveDZInput, radiusInput, angleDegInput, angleRadInput, curveLengthInput, stepsInput;
	double facetLength, pathDX, pathDY, pathDZ, curveX0, curveY0, curveZ0,
		curvedX, curvedY, curveDZ, radius, angleDeg, angleRad, curveLength, steps;
};