#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include "Facet_shared.h"
#include <vector>

class ImTexturePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
protected:
	Interface* mApp;
	InterfaceGeometry* interfGeom;
	void DrawTextureTable();
	std::string name = "Texture Plotter []###TexturePlotter";
	float dummyWidth = 0;
	bool autoSize = true;
	int viewIdx = 5, width=0, height=0;
	std::vector<std::vector<double>> data;
	void getData();
	size_t selFacetId;
	InterfaceFacet* selFacet;
	double maxValue;
	int maxX, maxY;
};