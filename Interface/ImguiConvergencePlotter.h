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
	std::vector<int> drawnFormulas;
	Interface* mApp;
	bool logY = false;
	bool colorBlind = false;
	float lineWidth = 2;
	std::string expression;
	void DrawConvergenceGraph();
};
