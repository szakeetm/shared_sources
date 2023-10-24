#include "ImguiSelectFacetByResult.h"
#include "imgui.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "../Helper/VectorHelper.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

bool ImSelectFacetByResult::Preprocess() {
	if (minHitsInput == "") state.push_back(noMinHits);
	else if (!Util::getNumber(&minHits, minHitsInput)) {
		ImIOWrappers::InfoPopup("Error", "Hits more than number invalid");
		return false;
	}
	if (maxHitsInput == "") state.push_back(noMaxHits);
	else if (!Util::getNumber(&maxHits, maxHitsInput)) {
		ImIOWrappers::InfoPopup("Error", "Hits less than number invalid");
		return false;
	}
	if (minAbsInput == "") state.push_back(noMinAbs);
	else if (!Util::getNumber(&minAbs, minAbsInput)) {
		ImIOWrappers::InfoPopup("Error", "Abs more than number invalid");
		return false;
	}
	if (maxAbsInput == "") state.push_back(noMaxAbs);
	else if (!Util::getNumber(&maxAbs, maxAbsInput)) {
		ImIOWrappers::InfoPopup("Error", "Abs less than number invalid");
		return false;
	}
	if (minDesInput == "") state.push_back(noMinDes);
	else if (!Util::getNumber(&minDes, minDesInput)) {
		ImIOWrappers::InfoPopup("Error", "Des more than number invalid");
		return false;
	}
	if (maxDesInput == "") state.push_back(noMaxDes);
	else if (!Util::getNumber(&maxDes, maxDesInput)) {
		ImIOWrappers::InfoPopup("Error", "Des less than number invalid");
		return false;
	}
	return true;
}

void ImSelectFacetByResult::Draw() {
	if (!drawn) return;
	static const float txtW = ImGui::CalcTextSize("0").x;
	ImGui::SetNextWindowPos(ImVec2(10, 20), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Select facets by simulation result", &drawn, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize)) {
		static const int width = 10;
		ImGui::Text("Empty texbox = condition ignored");
		if (ImGui::BeginTable("##SFBSR",3,ImGuiTableFlags_SizingFixedSame)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##minHits", &minHitsInput);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextCentered("< Hits <");
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##maxHits", &maxHitsInput);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##minAbs", &minAbsInput);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextCentered("< Abs <");
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##maxAbs", &maxAbsInput);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##minDes", &minDesInput);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextCentered("< Des <");
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##maxDes", &maxDesInput);

			ImGui::EndTable();
		}
		if (ImGui::Button("Select")) {
			state.push_back(btnSelect);
			DoSelect();
		}
		ImGui::SameLine();

		if (ImGui::Button("Add to sel.")) {
			state.push_back(btnAdd);
			DoSelect();
		}
		ImGui::SameLine();

		if (ImGui::Button("Remove from sel.")) {
			state.push_back(btnRmv);
			DoSelect();
		}
	}
	ImGui::End();
}

void ImSelectFacetByResult::Init(Interface* mApp_) {
	mApp = mApp_;
}

void ImSelectFacetByResult::DoSelect() {
	if (!Preprocess()) return;
	if (!mApp) throw std::runtime_error("mApp not initialized");
	InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
	if (Util::inVec(btnSelect, state)) interfGeom->UnselectAll();
	size_t nbFacet = interfGeom->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		InterfaceFacet* f = interfGeom->GetFacet(i);
		bool match = true;
		if (!Util::inVec(noMaxHits,state)) match = match && (f->facetHitCache.nbMCHit < maxHits);
		if (!Util::inVec(noMinHits,state)) match = match && (f->facetHitCache.nbMCHit > minHits);
		if (!Util::inVec(noMaxAbs, state)) match = match && (f->facetHitCache.nbAbsEquiv < maxAbs);
		if (!Util::inVec(noMinAbs, state)) match = match && (f->facetHitCache.nbAbsEquiv > minAbs);
		if (!Util::inVec(noMaxDes, state)) match = match && (f->facetHitCache.nbDesorbed < maxDes);
		if (!Util::inVec(noMinDes, state)) match = match && (f->facetHitCache.nbDesorbed > minDes);
		if (match) f->selected = (!Util::inVec(btnRmv, state));
	}
	interfGeom->UpdateSelection();
	mApp->UpdateFacetParams(true);
	mApp->UpdateFacetlistSelected();
	std::vector<states>().swap(state); // clear state
}