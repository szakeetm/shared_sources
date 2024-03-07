#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include <vector>

class ImConvergencePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp);
	void RemovePlot(int idx);
	void Reload();
	void LoadSettingsFromFile(bool log, std::vector<int> plotted);
	void DecrementFormulaIndicies(int startId);
protected:
	void MenuBar();
	bool Export(bool toFile = false, bool onlyVisible = false);
	int selectedFormula = -1;
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
	double startX=0, endX=1000;
	double step = 1.0;
	GLFormula formula;
	bool IsPlotted(size_t idx);
	bool lockYtoZero = false;
	int maxDatapoints = 1000;
	size_t actualNbValues = 0;
	bool showValueOnHover = true;
};
