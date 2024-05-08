#pragma once
#include "ImguiWindowBase.h"

class ImCollapse : public ImWindow {
public:
	void Draw();
protected:
	bool enableVertDistLimit = true;
	double vertDist = 1e-5;
	std::string vertDistIn = "1e-5";

	bool enableFacetCoplanarityLimit = true;
	double facetCoplanarity = 1e-5;
	std::string facetCoplanarityIn = "1e-5";

	bool enableFacetMaxVertexNum = false;
	double facetMaxVertNum = 100;
	std::string facetMaxVertNumIn = "100";

	bool enableFacetSideColinearityLimit = true;
	double facetSideColinearity = 1e-3;
	std::string facetSideColinearityIn = "1e-3";

	std::string message="Selected: 0\nVertex: 0\nFacet: 0", lastAction;
	void CollapseButtonPress(bool selectionOnly);
};