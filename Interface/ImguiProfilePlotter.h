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

	typedef struct {
		size_t facetID;
		std::vector<double> x;
		std::vector<double> y;
		ImVec4 color;
	} ImProfile;

	int viewIdx = 1;
	std::vector<ImProfile> data;
	ImProfile manualPlot;
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
};