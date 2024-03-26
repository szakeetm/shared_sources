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
	void PreProcessExtrude();
	void DoExtrude();
	int baseId=-1, dirId=-1, curveDirFac=-1;
	std::optional<size_t> AssertOneVertexSelected();
	std::optional<size_t> AssertOneFacetSelected();
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
		curveDXInput, curveDYInput, curveDZInput, radiusInput, angleDegInput, angleRadInput, curveLengthInput, stepsInput, facetInfo;
	double facetLength=1, pathDX, pathDY, pathDZ, curveX0, curveY0, curveZ0,
		curveDX, curveDY, curveDZ, radius, angleDeg, angleRad, curveLength, steps;
	friend class ImTest;
};