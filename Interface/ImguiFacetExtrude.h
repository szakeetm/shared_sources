#pragma once
#include "ImguiWindowBase.h"

class ImFacetExtrude : public ImWindow {
public:
	void Draw();
protected:
	void ExtrudeButtonPress();
	void GetBaseButtonPress();
	void GetDirectionButtonPress();
	void FacetCenterButtonPress();
	void FacetIndex1ButtonPress();
	void CurveGetBaseButtonPress();
	void CurveFacetUButtonPress();
	void CurveFacetVButtonPress();
	void CurveGetDirectionButtonPress();
	void FacetNXButtonPress();
	void FacetNYButtonPress();
	void FacetNZButtonPress();
	enum Mode : int {
		none,
		facetNormal,
		facetAntinormal,
		directionVector,
		curveNormal,
		curveAntinormal
	};
	Mode mode = none;
	std::string facetLengthInput="1", pathDXInput, pathDYInput, pathDZInput, curveX0Input, curveY0Input, curveZ0Input,
		curvedXInput, curvedYInput, curveDZInput, radiusInput, angleDegInput, angleRadInput, curveLengthInput, stepsInput;
	double facetLength=1, pathDX, pathDY, pathDZ, curveX0, curveY0, curveZ0,
		curvedX, curvedY, curveDZ, radius, angleDeg, angleRad, curveLength, steps;
};