#pragma once

#include "ImguiWindowBase.h"

class ImViewerSettings : public ImWindow {
public:
	void Draw();
protected:
	enum ShowFacetMode : int {
		front_back = 0,
		front = 1,
		back = 2
	};
	ShowFacetMode fMode = front_back;
	std::string comboText = "Front & Back";
	double transitionStep = 1.0, angleStep = 0.005;
	std::string transitionStepIn = "1.0", angleStepIn = "0.005";
	size_t numOfLines = 2048, numOfLeaks = 2048;
	std::string numOfLinesIn = "2048", numOfLeaksIn = "2048";
	bool showHiddenEdges = false, showHiddenVertex = true, showTextureGrid = false,
		largerHitDot = true, showTeleports = true, showTimeOverlay = false, hideUIByLimit = true;
	unsigned int hideUILimit = 500;
	std::string hideUILimitIn = "500";
	void CrossSectionButtonPress();
	bool showDirection = true;
	double normeRatio = 1;
	std::string normeRatioIn = "1";
	bool normalize = true, center = true;
	void ApplyButtonPress();
};