#pragma once
#include "ImguiWindowBase.h"

class ImMeasureForce : public ImWindow {
public:
	void Draw();
protected:
	bool enableForceMeasurement = false;
	void SelectedVertexButtonPress();
	void FacetCenterButtonPress();
	void ApplyButtonPress();
	void Apply();
	std::string mx0I = "0", my0I = "0", mz0I = "0";
	double mx = 0, my = 0, mz = 0;
};