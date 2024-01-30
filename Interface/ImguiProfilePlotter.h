#include <imgui.h>
#include "ImguiWindowBase.h"
#include "Geometry_shared.h"
#include <string>
#include <vector>
#include "../GLApp/GLFormula.h"
#include "Buffer_shared.h"

class ImProfilePlotter : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_);
	void LoadSettingsFromFile(bool log, std::vector<int> plotted);
	void Refresh();
	void UpdatePlotter();
private:
	void DrawProfileGraph();

	// button actions
	void ShowFacet();
	void AddCurve();
	void RemoveCurve(int id);
	void ComputeProfiles();
	void FacetHiglighting(bool toggle);
	void DrawMenuBar();
	bool Export(bool toFile = false);

	// helper / utility functions
	void UpdateSelection();
	bool IsPlotted(size_t facetId);

	std::vector<size_t> ParseManualFacetList();

	int viewIdx = 1;
	std::vector<ImPlotData> data;
	ImPlotData manualPlot;
	size_t profileSize = PROFILE_SIZE;

	bool correctForGas = false;
	bool updateHilights = false;

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
	double manualStart = 0, manualEnd = PROFILE_SIZE, manualStep = 0.1;
	bool lockYtoZero = false;
	bool showDatapoints = false;
	bool showValueOnHover = true;
	bool setLog = false;
	bool loading = false;
};