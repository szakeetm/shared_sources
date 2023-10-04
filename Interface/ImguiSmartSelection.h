#pragma once
#include <string>

class ImSmartSelection {
public:
	ImSmartSelection();
	void Draw();
	void Show();
	void Hide();
	void Toggle();
	bool IsVisible();
	bool IsToggled();
	double GetMaxAngle();
protected:
	std::string planeDiffInput;
	double planeDiff;
	bool isRunning;
	bool drawn;
	bool toggle;
	bool isAnalyzed;
	std::string result;
	void (*func)();
};