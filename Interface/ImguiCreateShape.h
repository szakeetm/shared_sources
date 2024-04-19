#pragma once
#include "ImguiWindowBase.h"

class ImCreateShape : public ImWindow {
public:
	void Draw();
protected:
	enum Shape {
		none,
		rect,
		elipse,
		track
	};
	Shape shapeSel = rect;

	double centerX, centerY, centerZ;
	std::string centerXIn, centerYIn, centerZIn;

	double axis1X, axis1Y, axis1Z;
	std::string axis1XIn, axis1YIn, axis1ZIn;

	double normalX, normalY, normalZ;
	std::string normalXIn, normalYIn, normalZIn;

	double axis1Len, axis2Len, trackTopLen, arcSteps;
	std::string axis1LenIn, axis2LenIn, trackTopLenIn, arcStepsIn;
};