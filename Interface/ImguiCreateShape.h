#pragma once
#include "ImguiWindowBase.h"
#include "ImguiImageManager.h"

class ImCreateShape : public ImWindow {
public:
	void Draw();
	void OnShow() override;
protected:
	enum Shape {
		none,
		rect,
		ellipse,
		track
	};
	Shape shapeSel = rect;

	double centerX = 0, centerY = 0, centerZ = 0;
	std::string centerXIn = "0", centerYIn = "0", centerZIn = "0";

	double axis1X = 0, axis1Y = 0, axis1Z = 0;
	std::string axis1XIn = "1", axis1YIn = "0", axis1ZIn = "0";

	double normalX = 0, normalY = 0, normalZ = 0;
	std::string normalXIn = "0", normalYIn = "0", normalZIn = "1";

	double axis1Len = 0, axis2Len = 0, trackTopLen = 0, arcSteps = 0;
	std::string axis1LenIn = "1", axis2LenIn = "1", trackTopLenIn, arcStepsIn = "10";
	
	void FacetCenterButtonPress();
	void VertexButtonPress();
	std::string centerRowMsg = "";
	void FacetUButtonPress();
	void CenterToVertAx1ButtonPress();
	std::string axisRowMsg = "";
	void FacetNButtonPress();
	void CenterToVertNormalButtonPress();
	std::string normalRowMsg = "";
	void FullCircleSidesButtonPress();
	void ApplyButtonPress();

	ImImage *rectImg = nullptr, *ellipseImg = nullptr, *trackImg = nullptr;
};