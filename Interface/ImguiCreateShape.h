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

	double centerX, centerY, centerZ;
	std::string centerXIn="0", centerYIn="0", centerZIn="0";

	double axis1X, axis1Y, axis1Z;
	std::string axis1XIn="1", axis1YIn="0", axis1ZIn="0";

	double normalX, normalY, normalZ;
	std::string normalXIn="0", normalYIn="0", normalZIn="1";

	double axis1Len, axis2Len, trackTopLen, arcSteps;
	std::string axis1LenIn="1", axis2LenIn="1", trackTopLenIn, arcStepsIn="10";
	
	void FacetCenterButtonPress();
	void VertexButtonPress();
	std::string centerRowMsg;
	void FacetUButtonPress();
	void CenterToVertAx1ButtonPress();
	std::string axisRowMsg;
	void FacetNButtonPress();
	void CenterToVertNormalButtonPress();
	std::string normalRowMsg;
	void FullCircleSidesButtonPress();
	void ApplyButtonPress();

	ImImage *rectImg, *ellipseImg, *trackImg;
};