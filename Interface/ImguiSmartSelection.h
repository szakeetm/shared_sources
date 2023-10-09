#pragma once
#include <string>

class ImSmartSelection {
public:
	void Init();
	void Draw();
	void Show();
	void Hide();
	bool IsVisible();
	
	void Toggle();
	bool IsToggled();
	double GetMaxAngle();
protected:
	std::string planeDiffInput = "30";
	double planeDiff = 30;
	bool isRunning = false;
	bool toggle = false;
	bool isAnalyzed = false;
	bool drawn = false;
	std::string result = "No neighborhood analysis yet.";
	void (*func)();
};