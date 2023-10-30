#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>

class ImTexturePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	Interface* mApp;
	InterfaceGeometry* interfGeom;
	void DrawTextureTable();
	std::string name = "Texture Plotter []";
	float dummyWidth = 0;
	bool autoSize = true;
	int selIdx = 5;
};