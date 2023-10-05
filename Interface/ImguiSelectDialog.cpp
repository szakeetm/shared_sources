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
ImSelectDialog::ImSelectDialog()
{
	select = []() {
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
		interfGeom->UnselectAll();
		for (auto facetId : mApp->imWnd->selByNum.facetIds) {
			interfGeom->GetFacet(facetId)->selected = true;
		}
		interfGeom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected();
	};
	addSelect = []() {
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
		for (auto facetId : mApp->imWnd->selByNum.facetIds) {
			interfGeom->GetFacet(facetId)->selected = true;
		}
		interfGeom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected(); 
	};
	rmvSelect = []() {
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
		for (auto facetId : mApp->imWnd->selByNum.facetIds) {
			interfGeom->GetFacet(facetId)->selected = false;
		}
		interfGeom->UpdateSelection();
		mApp->UpdateFacetParams(true);
		mApp->UpdateFacetlistSelected(); 
	};
	numText = "";
	drawn = false;
}

void ImSelectDialog::Draw()
{
	if (!drawn) return;
	ImGui::Begin("Select facet(s) by number", &drawn, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
	ImGui::Text("Facet number:"); ImGui::SameLine();
	ImGui::InputText("##1", &this->numText);
	ImGui::Text("You can enter a list and/or range(s), examples: 1,2,3 or 1-10 or 1-10,20-30");
	if (ImGui::Button("  Select  ")) {
		Preprocess();
		select();
	} ImGui::SameLine();
	if (ImGui::Button("  Add to selection  ")) {
		Preprocess();
		addSelect();
	} ImGui::SameLine();
	if (ImGui::Button("  Remove from selection  ")) {
		Preprocess();
		rmvSelect();
	}

	ImGui::End();
}

void ImSelectDialog::Show()
{
	drawn = true;
}

void ImSelectDialog::Preprocess() {
	facetIds.erase(facetIds.begin(),facetIds.end());
	try {
		InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
		splitFacetList(facetIds, numText, interfGeom->GetNbFacet());
	}
	catch (const std::exception& e) {
		mApp->imWnd->popup.Open("Error", e.what(), { std::make_shared<MyButtonInt>("Ok",buttonOk, ImGui::keyEnter) });
		return;
	}
}
