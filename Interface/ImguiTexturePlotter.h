#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include "Facet_shared.h"
#include <vector>
#include "imgui.h"

class ImTexturePlotter : public ImWindow {
public:
	void Draw();
	void Hide();
	void Init(Interface* mApp_);
	void UpdateOnFacetChange(const std::vector<size_t>& selectedFacets);
	void UpdatePlotter();
protected:
	// functions
	void OnShow() override;
	void DrawTextureTable();
	void GetData();
	bool IsCellSelected(size_t row, size_t col);
	void SelectRow(size_t row);
	void SelectColumn(size_t col);
	void BoxSelect(const std::pair<int, int>& start, const std::pair<int, int>& end);
	ImVec4 SelectionBounds();
	bool SaveTexturePlotter(bool toFile=true);
	void DrawMenuBar();
	struct SelRect { int startRow, startCol, endRow, endCol; };
	std::string Serialize(SelRect bounds = { 0,0,0,0 }, char lineBreak = '\n', std::string rowBreak = "\t");

	// UI variables
	bool isDragging = false;
	float dummyWidth = 0;
	std::string name = "Texture Plotter []###TexturePlotter";
	std::vector<std::string> comboOpts = { u8"Cell Area [cm\u00B2]###0", "# of MC hits###1", u8"Impingement rate [1 / m\u00B2 / sec]]###2", u8"Particle density [1 / m\u00B3]###3", u8"Gas density [kg / m\u00B3]###4", "Pressure [mBar]###5", "Avg.speed estimate [m / s]###6", "Incident velocity vector [m / s]###7", "# of velocity vectors###8" };
	int viewIdx = 5;
	std::vector<std::pair<int,int>> selection;
	bool selectionChanged = false;
	bool resizableColumns = false;
	bool fitToWindow = false;

	// app + data variables
	InterfaceGeometry* interfGeom;
	InterfaceFacet* selFacet;
	int width = 0, height = 0; // table size
	std::vector<std::vector<std::string>> data; // 2d vector for values
	size_t selFacetId;
	double maxValue; // value of max value
	size_t maxX, maxY; // column and row of max value
	int tableFlags = 0;
	int columnWidth; // column width (is multiplied by character width)
	bool scrollToSelected = false;
	size_t profSize = 0;
	bool wasDrawn = false;
	friend class ImTest;
};