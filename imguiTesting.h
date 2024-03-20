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
	void RunTests();
	bool running = false;
	int ranScenarios = 0;
	bool result = false;
	
	enum Configuration : int {
		empty = 0,
		qPipe = 1,
		profile = 2
	};
	int areScenarios = 3;
	Configuration currentConfig = empty;
	bool ConfigureGeometry(Configuration index = empty);

	void DrawPresetControl();

	void SelectFacet(size_t idx, bool shift = false, bool ctrl = false);
	void SelectFacet(std::vector<size_t> idxs, bool shift = false , bool ctrl = false);
	void SelectVertex(size_t idx, bool add = false);
	void SelectVertex(std::vector<size_t> idxs, bool add = false);
	bool SetFacetProfile(size_t facetIdx, int profile);

private:
	std::queue<std::function<void()>> callQueue;
	void RegisterTests();
	ImGuiTestEngine* engine = nullptr;
};