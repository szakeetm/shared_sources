#pragma once
#include "ImguiWindowBase.h"

class ImMeasureForce : public ImWindow {
public:
	void Draw();
protected:
	bool enableForceMeasurement = false;
	std::string mx0I = "0", my0I = "0", mz0I = "0";
};