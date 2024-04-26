#pragma once
#include "ImguiWindowBase.h"

class ImOutgassingMap : public ImWindow {
public:
	void Draw();
	void UpdateOnFacetChange(const std::vector<size_t>& selectedFacets);
protected:
	std::string name = "Outgassing map";
	void ExplodeButtonPress();
	void AutosizeButtonPress();
	void PasteButtonPress();
	void DrawTable();
	void OnShow() override;
	size_t selFacetId;
	enum DesorptionType {
		uniform,
		cosine,
		cosineN
	};
	DesorptionType desorptionType = cosine;
	std::string comboText = "Cosine";

	float exponent;
	std::string exponentIn;
};