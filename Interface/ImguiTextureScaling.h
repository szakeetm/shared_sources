#include "ImguiWindowBase.h"

class ImTextureScailing : public ImWindow {
public:
	void Draw();
protected:
	std::string minInput = "0", maxInput = "1";
	double min = 0, max = 1;
	bool autoscale = true, colors = true, logScale = true;
	short includeComboVal = 0, showComboVal=0;
	std::string swapText = "0 bytes";
};