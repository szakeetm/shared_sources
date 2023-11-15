#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include <vector>

class ImConvergencePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp);
protected:
	void MenuBar();
	bool Export(bool toFile = false);

	bool showDatapoints = false;
	std::vector<ImPlotData> data;
	Interface* mApp;
	bool logY = false;
	bool colorBlind = false;
	float lineWidth = 2;
	std::string expression;
	void DrawConvergenceGraph();
	bool drawManual = false;
	std::vector<double> manualxValues, manualyValues;
	int startX=0, endX=1000;
	double step = 1.0;
	GLFormula formula;
	bool IsPlotted(size_t idx);
	bool lockYtoZero = false;
};
