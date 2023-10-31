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
	// functions
	void DrawTextureTable();
	void getData();
	bool IsCellSelected(size_t row, size_t col);
	void SelectRow(size_t row);
	void SelectColumn(size_t col);
	void DragSelect();
	
	// UI variables
	float dummyWidth = 0;
	bool autoSize = true;
	std::string name = "Texture Plotter []###TexturePlotter";
	int viewIdx = 5;
	std::vector<std::pair<int,int>> selection;
	// app + data variables
	Interface* mApp;
	InterfaceGeometry* interfGeom;
	InterfaceFacet* selFacet;
	size_t width = 0, height = 0;
	std::vector<std::vector<std::string>> data;
	size_t selFacetId;
	double maxValue;
	size_t maxX, maxY;
};