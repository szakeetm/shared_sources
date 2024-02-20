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
	
private:
	void RegisterTests();
	ImGuiTestEngine* engine;
};