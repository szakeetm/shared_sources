#pragma once
#include <string>
#include "ImguiWindowBase.h"

class ImSmartSelection : public ImWindow{
public:
	void Func();
	void Draw();
	
	bool IsEnabled();
	double GetMaxAngle();
protected:
	std::string planeDiffInput = "30";
	double planeDiff = 30;
	bool isRunning = false;
	bool enabledToggle = false;
	bool isAnalyzed = false;
	std::string result = "No neighborhood analysis yet.";
};