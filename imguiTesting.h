#pragma once
#include "imgui_test_engine/imgui_te_engine.h"
#include "imgui_test_engine/imgui_te_ui.h"
#include "imgui_test_engine//imgui_te_context.h"
#include "ImguiWindowBase.h"

class ImTest : public ImWindow {
public:
	void Init(Interface* mApp_);
	void Draw();

	bool StopEngine();
	bool DestroyContext();
	void PostSwap();
	
	enum Configuration : int {
		empty = 0,
		qPipe = 1
	};
	bool ConfigureGeometry(Configuration index = empty);

	void SelectFacet(size_t idx, bool shift = false, bool ctrl = false);
	void SelectFacet(std::vector<size_t> idxs, bool shift = false , bool ctrl = false);

private:
	void RegisterTests();
	ImGuiTestEngine* engine = nullptr;
};