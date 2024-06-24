#pragma once
#include "ImguiWindowBase.h"
#include "Facet_shared.h"

class ImFacetSplit : public ImWindow {
public:
	void Draw();
	void Reset();
protected:
	enum Mode {
		none,
		equation,
		facet,
		vertices
	};
	Mode mode = none;
	std::string xIn = "", yIn = "", zIn = "", wIn = "", fIdIn = "";
	double x = 0, y = 0, z = 0, w = 0;
	std::string output = "";
	size_t facetId = 0, nbFacet = 0, nbCreated = 0;
	bool enableUndo = false;
	std::vector<DeletedFacet> deletedFacetList;

	void SplitButtonPress();
	void UndoButtonPress();
	void DoUndo();
	friend class ImTest;
};