#include "imgui.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "Interface.h"
#include "ImguiWindow.h"
#include "ImguiPopup.h"
#include "Geometry_shared.h"
#include "Helper/GLProgress_ImGui.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

void ImSmartSelection::Func() {
	if (!isRunning) {
		if (!Util::getNumber(&planeDiff, planeDiffInput)) {
			ImIOWrappers::InfoPopup("Error", "Invalid input");
			return;
		}
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
		if (!interfGeom->IsLoaded()) {
			mApp->imWnd->popup.Open("Error", "No geometry", { 
				std::make_shared<ImIOWrappers::ImButtonInt>("Ok", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
				});
			return;
		}
		isRunning = true;
		size_t nbAnalyzed;
		{
			LockWrapper lW(mApp->imguiRenderLock);
			auto prg = GLProgress_GUI("Analyzing facets...", "Please wait");
			prg.SetVisible(true);
			nbAnalyzed = interfGeom->AnalyzeNeigbors(&mApp->worker, prg);
		}
		isRunning = false;
		result = "Analyzed "+std::to_string(nbAnalyzed)+" facets.";
		enabledToggle = true;
		isAnalyzed = true;
	}
	else {
		isRunning = false;
		mApp->worker.abortRequested = true;
	}
}

void ImSmartSelection::Draw()
{
	if (drawn) {
		ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
		ImGui::Begin("Smart Selection", &drawn, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
		ImGui::PlaceAtRegionCenter(!isRunning ? "  Analyze  " : "  Stop Analyzing  ");
		if (ImGui::Button(!isRunning ? "  Analyze  ###Analyze" : "  Stop Analyzing  ")) {
			Func();
		}
		ImGui::Text("Max plane diff. between neighbors (deg):"); ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("000000").x);
		ImGui::InputText("##planeDiff", &planeDiffInput);
		ImGui::Text(result);
		if (!isAnalyzed) {
			ImGui::BeginDisabled();
		}
		
		ImGui::Checkbox("Enable smart selection", &this->enabledToggle);

		if (!isAnalyzed) ImGui::EndDisabled();
		ImGui::End();
	}
}

bool ImSmartSelection::IsEnabled()
{
	return this->enabledToggle;
}

double ImSmartSelection::GetMaxAngle()
{
	if (!IsVisible() || !IsEnabled()) return -1.0;
	if (Util::getNumber(&this->planeDiff, this->planeDiffInput)) {
		return this->planeDiff / 180.0 * 3.14159;
	}
	mApp->imWnd->popup.Open("Smart Select Error", "Invalid angle threshold in Smart Selection dialog\nMust be a non-negative number.", {
		std::make_shared<ImIOWrappers::ImButtonInt>("Ok", ImIOWrappers::buttonOk, ImGuiKey_Enter)
		});
	return -1.0;
}
