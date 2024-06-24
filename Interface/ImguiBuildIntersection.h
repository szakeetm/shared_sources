#pragma once

#include "ImguiWindowBase.h"
#include "Facet_shared.h"

class ImBuildIntersect : public ImWindow {
public:
	void Draw();
protected:
	void BuildButtonPress();
	void UndoButtonPress();
	void ClearUndoFacets();
	std::string message = "";
	std::vector<DeletedFacet> deletedFacetList;
	size_t nbFacet = 0, nbCreated = 0;
	bool undoEnabled = false;
};