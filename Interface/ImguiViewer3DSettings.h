#pragma once

#include "ImguiWindowBase.h"

class ImViewerSettings : public ImWindow {
public:
	void Draw();
	void SetPos(ImVec2 pos);
	void Update();
	void OnShow() override;
protected:
	enum ShowFacetMode : int {
		front_back = 0,
		front = 1,
		back = 2
	};
	ShowFacetMode fMode = front_back;
	std::string comboText = "Front & Back";
	double translationStep = 1.0, angleStep = 0.005;
	std::string translationStepIn = "1.0", angleStepIn = "0.005";
	size_t numOfLines = 2048, numOfLeaks = 2048;
	std::string numOfLinesIn = "2048", numOfLeaksIn = "2048";
	bool showHiddenEdges = false, showHiddenVertex = true, showTextureGrid = false,
		largerHitDot = true, showTeleports = true, showTimeOverlay = false, supressUI = true;
	unsigned int supressUILimit = 500;
	std::string supressUILimitIn = "500";
	void CrossSectionButtonPress();
	bool showDirection = true;
	double normeRatio = 1;
	std::string normeRatioIn = "1";
	bool normalize = true, center = true;
	void ApplyButtonPress();
	bool positionChangePending = false;
	ImVec2 newPos;
	GeometryViewer* viewer;
	std::string title = "Viewer";
};