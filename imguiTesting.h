#pragma once
#include "imgui_test_engine/imgui_te_engine.h"
#include "imgui_test_engine/imgui_te_ui.h"
#include "imgui_test_engine//imgui_te_context.h"
#include "ImguiWindowBase.h"
#include <functional>

class ImTest : public ImWindow {
public:
	void Init(Interface* mApp_);
	void Draw();

	bool StopEngine();
	bool DestroyContext();
	void PostSwap();
	
	void DrawPresetControl();

	void RunTests(); // autorun tests (used when running from command line)
private:
	// variables for keeping track of tests when running from command line
	bool running = false;
	int ranScenarios = 0;
	bool result = false;

	// switching between scenarios (defined in ConfigureGeometry
	enum Configuration : int {
		empty = 0,
		qPipe = 1,
		profile = 2
	};
	int numberOfScenarios = 3;
	Configuration currentConfig = empty;
	bool SetFacetProfile(size_t facetIdx, int profile);

	bool ConfigureGeometry(Configuration index = empty);

	// wrapper functions for changing selections mid-test (needs to enqueue functions to be executed outside of the test body)
	void SelectFacet(size_t idx, bool shift = false, bool ctrl = false);
	void SelectFacet(std::vector<size_t> idxs, bool shift = false , bool ctrl = false);
	void SelectVertex(size_t idx, bool add = false);
	void SelectVertex(std::vector<size_t> idxs, bool add = false);
	void DeselectAll();
	void DeselectAllVerticies();
	// -----
	std::queue<std::function<void()>> callQueue; // queue for calls to interface geometry to be executed mid-test but outside of test body
	void RegisterTests(); // runs on startup, contains test definitions
	ImGuiTestEngine* engine = nullptr;
};