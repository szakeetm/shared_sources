#include "ImguiSmartSelection.h"
#include "imgui/imgui.h"
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

void ImSmartSelection::Init() {
	func = []() {
		if (!mApp->imWnd->smartSelect.isRunning) {
			InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
			if (!interfGeom->IsLoaded()) {
				mApp->imWnd->popup.Open("Error", "No geometry", { 
					std::make_shared<WrappersIO::MyButtonInt>("Ok", WrappersIO::buttonOk, SDL_SCANCODE_RETURN) 
					});
				return;
			}
			mApp->imWnd->smartSelect.isRunning = true;
			size_t nbAnalyzed = interfGeom->AnalyzeNeigbors(&mApp->worker, mApp->imWnd->progress);
			mApp->imWnd->smartSelect.isRunning = false;
			mApp->imWnd->smartSelect.result = "Analyzed "+std::to_string(nbAnalyzed)+" facets.";
			mApp->imWnd->smartSelect.enabledToggle = true;
			mApp->imWnd->smartSelect.isAnalyzed = true;

		}
		else {
			mApp->imWnd->smartSelect.isRunning = false;
			mApp->worker.abortRequested = true;
		}
	};
}

void ImSmartSelection::Draw()
{
	if (drawn) {
		ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
		ImGui::Begin("Smart Selection", &drawn, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
		ImGui::PlaceAtRegionCenter(!isRunning ? "  Analyze  " : "  Stop Analyzing  ");
		if (ImGui::Button(!isRunning ? "  Analyze  " : "  Stop Analyzing  ")) {
			func();
		}
		ImGui::Text("Max plane diff. between neighbors (deg):"); ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcTextSize("000000").x);
		ImGui::InputText("##1", &planeDiffInput);
		ImGui::Text(result);
		if (!isAnalyzed) {
			ImGui::BeginDisabled();
		}
		
		ImGui::Checkbox("Enable smart selection", &this->enabledToggle);

		if (!isAnalyzed) ImGui::EndDisabled();
		ImGui::End();
	}
}

void ImSmartSelection::Show()
{
	drawn = true;
}

void ImSmartSelection::Hide()
{
	drawn = false;
}

void ImSmartSelection::Toggle()
{
	drawn = !drawn;
}

bool ImSmartSelection::IsVisible()
{
	return drawn;
}

bool ImSmartSelection::IsEnabled()
{
	return this->enabledToggle;
}

double ImSmartSelection::GetMaxAngle()
{
	if (Util::getNumber(&this->planeDiff, this->planeDiffInput) && planeDiff>=0.0) {
		return this->planeDiff / 180.0 * 3.14159;
	}
	mApp->imWnd->popup.Open("Smart Select Error", "Invalid angle threshold in Smart Selection dialog\nMust be a non-negative number.", {
		std::make_shared<WrappersIO::MyButtonInt>("Ok", WrappersIO::buttonOk, SDL_SCANCODE_RETURN)
		});
	return -1.0;
}
