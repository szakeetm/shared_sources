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
	bool PreProcessExtrude();
	void DoExtrude();
	size_t baseId = 0, dirId = 0, curveDirFac = 0;
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
	Mode mode = facetNormal;
	std::string facetLengthInput="1", pathDXInput="", pathDYInput = "", pathDZInput = "", curveX0Input = "", curveY0Input = "", curveZ0Input = "",
		curveDXInput = "", curveDYInput = "", curveDZInput = "", radiusInput = "", angleDegInput = "", angleRadInput = "", curveLengthInput = "", stepsInput = "", facetInfo = "";
	double facetLength = 1, pathDX=0, pathDY = 0, pathDZ = 0, curveX0 = 0, curveY0 = 0, curveZ0 = 0,
		curveDX = 0, curveDY = 0, curveDZ = 0, radius = 0, angleDeg = 0, angleRad = 0, curveLength = 0;
	size_t steps = 8;
	friend class ImTest;
};