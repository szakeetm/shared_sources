#include "ImguiSelectDialog.h"
#include "ImguiWindow.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Interface.h"
#include "Geometry_shared.h"
#include "ImguiExtensions.h"
#include "Facet_shared.h"
#include "imgui_stdlib/imgui_stdlib.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

void ImSelectDialog::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::Begin("Select facet(s) by number", &drawn, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Facet number:"); ImGui::SameLine();
	ImGui::InputText("##1", &this->numText);
	ImGui::Text("You can enter a list and/or range(s), examples: 1,2,3 or 1-10 or 1-10,20-30");
	if (ImGui::Button("  Select  ")) {
		Preprocess();
		Func(select);
	} ImGui::SameLine();
	if (ImGui::Button("  Add to selection  ")) {
		Preprocess();
		Func(addSelect);
	} ImGui::SameLine();
	if (ImGui::Button("  Remove from selection  ")) {
		Preprocess();
		Func(rmvSelect);
	}

	ImGui::End();
}

void ImSelectDialog::Preprocess() {
	facetIds.clear();

	try {
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
		splitFacetList(facetIds, numText, interfGeom->GetNbFacet());
	}
	catch (const std::exception& e) {
		mApp->imWnd->popup.Open("Error", e.what(), { 
			std::make_shared<ImIOWrappers::ImButtonInt>("Ok",ImIOWrappers::buttonOk, SDL_SCANCODE_RETURN) 
			});
		return;
	}
}

void ImSelectDialog::Func(int mode) {
	InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
	if (mode == select) {
		interfGeom->UnselectAll();
	}
	for (auto facetId : facetIds) {
		interfGeom->GetFacet(facetId)->selected = mode != rmvSelect;
	}
	interfGeom->UpdateSelection();
	mApp->UpdateFacetParams(true);
	mApp->UpdateFacetlistSelected();
}