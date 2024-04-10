#include "ImguiFacetSplit.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Helper/StringHelper.h"
#include "Facet_shared.h"

void ImFacetSplit::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 11.3));
	ImGui::Begin("Split Facet", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	ImGui::BeginChild("###FSPDM", ImVec2(0, ImGui::GetContentRegionAvail().y - txtH * 1.3), ImGuiChildFlags_Border);
	ImGui::TextDisabled("Plane definition mode");
	if (ImGui::RadioButton("By equation", mode == Mode::equation)) mode = Mode::equation;
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqX", &xIn);
	ImGui::AlignTextToFramePadding();
	ImGui::SameLine();
	ImGui::Text("*X +");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqY", &yIn);
	ImGui::SameLine();
	ImGui::Text("*Y +");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqZ", &zIn);
	ImGui::SameLine();
	ImGui::Text("*Z +");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqW", &wIn);
	ImGui::SameLine();
	ImGui::Text("= 0");
	if(ImGui::Button("XY plane")) {
		mode = Mode::equation;
		xIn = "0";
		yIn = "0";
		zIn = "1";
		wIn = "0";
	} ImGui::SameLine();
	if(ImGui::Button("YZ plane")) {
		mode = Mode::equation;
		xIn = "1";
		yIn = "0";
		zIn = "0";
		wIn = "0";
	} ImGui::SameLine();
	if(ImGui::Button("XZ plane")) {
		mode = Mode::equation;
		xIn = "0";
		yIn = "1";
		zIn = "0";
		wIn = "0";
	}
	if (ImGui::RadioButton("Plane of facet", mode == Mode::facet)) mode = Mode::facet;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW*6);
	ImGui::InputText("###fId", &fIdIn);
	ImGui::SameLine();
	if (ImGui::Button("<- Get selected")) {
		if (interfGeom->GetNbSelectedFacets() != 1) {
			ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		}
		else
		{
			facetId = interfGeom->GetSelectedFacets()[0];
			fIdIn = fmt::format("{}", facetId);
		}
	}
	if (ImGui::RadioButton("By 3 selected verticies", mode == Mode::verticies)) mode = Mode::verticies;
	ImGui::EndChild();
	if (ImGui::Button("Split")) {
		// todo
	} ImGui::SameLine();
	bool disable = !enableUndo;
	if (disable) ImGui::BeginDisabled();
	if (ImGui::Button("Undo")) {
		// todo
	}
	if (disable) ImGui::EndDisabled();
	ImGui::End();
}

void ImFacetSplit::SplitButtonPress()
{
	// validate
	switch (mode) {
	case none:
		ImIOWrappers::InfoPopup("Error", "Select plane definition mode");
		break;
	case equation:
		if (!Util::getNumber(&x, xIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid A coefficient");
			return;
		}
		if (!Util::getNumber(&y, yIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid B coefficient");
			return;
		}
		if (!Util::getNumber(&z, zIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid C coefficient");
			return;
		}
		if (!Util::getNumber(&w, wIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid D coefficient");
			return;
		}
		if (x == y == z == 0) {
			ImIOWrappers::InfoPopup("Error", "A B and C are all 0, this is not a valid plane definition");
			return;
		}
		break;
	case facet:
		if (!Util::getNumber(&facetId, fIdIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid number in facet ID");
			return;
		}
		if (facetId > interfGeom->GetNbFacet()) {
			ImIOWrappers::InfoPopup("Error", "No such facet");
			return;
		}
		break;
	case verticies:
		if (interfGeom->GetNbSelectedVertex() != 3) {
			ImIOWrappers::InfoPopup("Error", "Select exactly 3 verticies");
			return;
		}
		break;
	default:
		break;
	}
	// perform split
	// todo
}

void ImFacetSplit::UndoButtonPress()
{
	//todo
}
