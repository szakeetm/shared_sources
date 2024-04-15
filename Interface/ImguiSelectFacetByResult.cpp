#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS#include "imgui.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiSelectFacetByResult.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "Helper/MathTools.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif
// TODO Synrad stuff

bool ImSelectFacetByResult::Preprocess() {
	if (minHitsInput == "") states.push_back(noMinHits);
	else if (!Util::getNumber(&minHits, minHitsInput)) {
		ImIOWrappers::InfoPopup("Error", "Hits more than number invalid");
		return false;
	}
	if (maxHitsInput == "") states.push_back(noMaxHits);
	else if (!Util::getNumber(&maxHits, maxHitsInput)) {
		ImIOWrappers::InfoPopup("Error", "Hits less than number invalid");
		return false;
	}
	if (minAbsInput == "") states.push_back(noMinAbs);
	else if (!Util::getNumber(&minAbs, minAbsInput)) {
		ImIOWrappers::InfoPopup("Error", "Abs more than number invalid");
		return false;
	}
	if (maxAbsInput == "") states.push_back(noMaxAbs);
	else if (!Util::getNumber(&maxAbs, maxAbsInput)) {
		ImIOWrappers::InfoPopup("Error", "Abs less than number invalid");
		return false;
	}
	if (minDesInput == "") states.push_back(noMinDes);
	else if (!Util::getNumber(&minDes, minDesInput)) {
		ImIOWrappers::InfoPopup("Error", "Des more than number invalid");
		return false;
	}
	if (maxDesInput == "") states.push_back(noMaxDes);
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
		ImGui::Text("Empty textbox = condition ignored");
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
#ifdef MOLFLOW
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##minDes", &minDesInput);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextCentered("< Des <");
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * width);
			ImGui::InputText("##maxDes", &maxDesInput);
#endif
			// TODO Synrad specific fields
			ImGui::EndTable();
		}
		if (ImGui::Button("Select")) {
			states.push_back(btnSelect);
			DoSelect();
		}
		ImGui::SameLine();

		if (ImGui::Button("Add to sel.")) {
			states.push_back(btnAdd);
			DoSelect();
		}
		ImGui::SameLine();

		if (ImGui::Button("Remove from sel.")) {
			states.push_back(btnRmv);
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
	if (Contains(states, btnSelect)) interfGeom->UnselectAll();
	size_t nbFacet = interfGeom->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		InterfaceFacet* f = interfGeom->GetFacet(i);
		bool match = true;
		if (!Contains(states, noMaxHits)) match = match && (f->facetHitCache.nbMCHit < maxHits);
		if (!Contains(states, noMinHits)) match = match && (f->facetHitCache.nbMCHit > minHits);
		if (!Contains(states, noMaxAbs)) match = match && (f->facetHitCache.nbAbsEquiv < maxAbs);
		if (!Contains(states, noMinAbs)) match = match && (f->facetHitCache.nbAbsEquiv > minAbs);
#ifdef MOLFLOW
		if (!Contains(states, noMaxDes)) match = match && (f->facetHitCache.nbDesorbed < maxDes);
		if (!Contains(states, noMinDes)) match = match && (f->facetHitCache.nbDesorbed > minDes);
#endif
		// TODO Synrad specific fields
		if (match) f->selected = (!Contains(states, btnRmv));
	}
	interfGeom->UpdateSelection();
	mApp->UpdateFacetParams(true);
	mApp->UpdateFacetlistSelected();
	std::vector<state>().swap(states); // clear states
}