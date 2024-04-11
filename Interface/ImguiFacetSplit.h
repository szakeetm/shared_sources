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
		verticies
	};
	Mode mode = none;
	std::string xIn, yIn, zIn, wIn, fIdIn;
	double x, y, z, w;
	std::string output;
	size_t facetId, nbFacet, nbCreated;
	bool enableUndo = false;
	std::vector<DeletedFacet> deletedFacetList;

	void SplitButtonPress();
	void UndoButtonPress();
	void DoUndo();
};