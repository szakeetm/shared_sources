#pragma once
#include "ImguiWindowBase.h"
#include "Facet_shared.h"

class ImOutgassingMap : public ImWindow {
public:
	void Draw();
	void UpdateOnFacetChange(const std::vector<size_t>& selectedFacets);
protected:
	std::string name = "Outgassing map";
	void ExplodeButtonPress();
	void PasteButtonPress();
	void DrawTable();
	void UpdateData();
	std::vector<std::vector<std::string>> data;
	void OnShow() override;
	size_t selFacetId = 0;
	InterfaceFacet* facet = 0;
	enum DesorptionType : int {
		uniform = 0,
		cosine = 1,
		cosineN = 2
	};
	struct Selection { 
		size_t row = 0, column = 0; 
		bool active = false, changed = false; 
	} selection;
	DesorptionType desorptionType = cosine;
	std::string comboText = "Cosine";

	std::string exponentIn;
};