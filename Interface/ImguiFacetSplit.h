#pragma once
#include "ImguiWindowBase.h"

class ImFacetSplit : public ImWindow {
public:
	void Draw();
protected:
	enum Mode {
		none,
		equation,
		facet,
		verticies
	};
	Mode mode = none;
	std::string xIn, yIn, zIn, wIn, fIdIn;
	double x, y, z, w;
	size_t facetId;
	bool enableUndo = false;

	void SplitButtonPress();
	void UndoButtonPress();
};