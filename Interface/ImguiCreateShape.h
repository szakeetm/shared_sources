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

	ImImage *rectImg, *elipseImg, *trackImg;
};