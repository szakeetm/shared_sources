#include "ImguiWindowBase.h"
#include "Geometry_shared.h"
#include <string>

class ImProfilePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
private:
	void DrawProfileGraph();
	bool PlotNewExpression();
	void computeManual();
	InterfaceGeometry* interfGeom;
	int nProfileFacets = 0;
	std::string manualFacetSel;
	bool colorBlind = false, identProfilesInGeom = false;
	float lineWidth = 2;
	std::string expression;
	double step = 1.0;
	int startX = 0, endX = 1000;
	bool drawManual = false;
};