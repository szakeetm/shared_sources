#pragma once
#include <string>

class ImSmartSelection {
public:
	void Func();
	void Draw();
	void Show();
	void Hide();
	bool IsVisible();
	
	void Toggle();
	bool IsEnabled();
	double GetMaxAngle();
protected:
	std::string planeDiffInput = "30";
	double planeDiff = 30;
	bool isRunning = false;
	bool enabledToggle = false;
	bool isAnalyzed = false;
	bool drawn = false;
	std::string result = "No neighborhood analysis yet.";
};