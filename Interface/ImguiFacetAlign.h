#include "ImguiWindowBase.h"

class ImFacetAlign : public ImWindow {
public:
	void Draw();
protected:
	bool invertNormal, swapOnSource, swapOnDestination;
	std::vector<int> toAlign;
};