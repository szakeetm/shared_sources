

#pragma once
#include "ImguiWindowBase.h"

class ImguiWindow;
namespace ImMenu {
	void ConvergencePlotterMenuPress();
};
void ShowAppMainMenuBar();
void RegisterShortcuts();

class ImExplodeFacet : public ImWindow {
public:
	void Draw();
private:
	std::string input;
	double value;
	void DoExplode();
};