#pragma once
#include <string>

class ImSmartSelection {
public:
	ImSmartSelection();
	
	void Draw();
	void Show();
	void Hide();
	bool IsVisible();
	
	void Toggle();
	bool IsToggled();
	double GetMaxAngle();
protected:
	std::string planeDiffInput;
	double planeDiff;
	bool isRunning;
	bool toggle;
	bool isAnalyzed;
	bool drawn;
	std::string result;
	void (*func)();
};