#include <imgui.h>
#include "ImguiWindowBase.h"
#include "Geometry_shared.h"
#include <string>
#include <vector>
#include "GLFormula.h"

class ImProfilePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
private:
	void DrawProfileGraph();
	void DrawValueOnHover();

	// button actions
	void ShowFacet();
	void AddCurve();
	void RemoveCurve();
	void computeProfiles();
	void FacetHiglighting(bool toggle);

	// helper / utility functions
	void UpdateSelection();
	bool IsPlotted(size_t facetId);

	std::vector<size_t> manualFacetList();

	int viewIdx = 1;
	std::vector<ImPlotData> data;
	ImPlotData manualPlot;
	size_t profileSize = 100;

	bool correctForGas = false;

	InterfaceGeometry* interfGeom;
	size_t selectedProfile = -1;
	InterfaceFacet* f = 0;
	int nProfileFacets = 0;
	std::string manualFacetSel;
	bool colorBlind = false, identProfilesInGeom = false;
	float lineWidth = 2;
	std::string expression;
	GLFormula formula;
	bool drawManual = false;
	bool lockYtoZero = false;
};