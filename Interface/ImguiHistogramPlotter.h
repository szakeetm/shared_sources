#pragma once
#include "ImguiWindowBase.h"

class ImHistogramPlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	//functions
	void DrawPlot();

	//types
	enum plotTabs {bounces, distance, time};
	
	class ImHistagramSettings : public ImWindow {
	public:
		void Draw();
		float width;
	};

	//variables
	ImHistagramSettings settingsWindow;
	plotTabs plotTab = bounces;
	std::string xAxisName = "Number of bounces";
	bool normalize = false;
};