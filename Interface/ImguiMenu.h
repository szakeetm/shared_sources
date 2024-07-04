#pragma once
#include "ImguiWindowBase.h"

class ImguiWindow;
namespace ImMenu {
	void ConvergencePlotterMenuPress();
	void NewGeometry();
	void ShowAppMainMenuBar();
	void RegisterShortcuts();
};

class ImExplodeFacet : public ImWindow {
public:
	void Draw();
private:
	std::string input = "";
	double value = 0;
	void DoExplode();
};