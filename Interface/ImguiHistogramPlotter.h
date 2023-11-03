#include "ImguiWindowBase.h"

class ImHistogramPlotter : public ImWindow {
public:
	void Draw();
protected:
	//functions
	void DrawPlot();

	//types
	enum plotTabs {bounces, distance, time};
	
	//variables
	plotTabs plotTab = bounces;
	std::string xAxisName = "Number of bounces";
	bool normalize = false;
};