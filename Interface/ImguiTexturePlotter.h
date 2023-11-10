#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include "Facet_shared.h"
#include <vector>
#include <imgui.h>

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
	void BoxSelect(const std::pair<int, int>& start, const std::pair<int, int>& end);
	ImVec4 SelectionBounds();
	bool SaveToFile();

	// UI variables
	bool isDragging = false;
	float dummyWidth = 0;
	std::string name = "Texture Plotter []###TexturePlotter";
	int viewIdx = 5;
	std::vector<std::pair<int,int>> selection;
	
	// app + data variables
	InterfaceGeometry* interfGeom;
	InterfaceFacet* selFacet;
	size_t width = 0, height = 0; // table size
	std::vector<std::vector<std::string>> data; // 2d vector for values
	size_t selFacetId;
	double maxValue; // value of max value
	size_t maxX, maxY; // column and row of max value
	int columnFlags = 0;
	int tableFlags = 0;
	int columnWidth; // column width (is multiplied by character width)
	bool scrollToMax = false;
};