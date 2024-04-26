#include "ImguiOutgassingMap.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImOutgassingMap::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW*70, txtH*20), ImGuiCond_FirstUseEver);
	ImGui::Begin((name + "###OgM").c_str(), &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("OgMTab", ImVec2(0,ImGui::GetContentRegionAvail().y-txtH*1.5), ImGuiChildFlags_Border);
	DrawTable();
	ImGui::EndChild();
	
	if (ImGui::Button("Explode")) {
		ExplodeButtonPress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Auto size")) {
		AutosizeButtonPress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Paste")) {
		PasteButtonPress();
	}
	ImGui::SameLine();
	ImGui::Text("Desporption type:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 13);
	if (ImGui::BeginCombo("###OMDT", comboText)) {
		if (ImGui::Selectable("Uniform")) {
			comboText = "Uniform";
			desorptionType = uniform;
		}
		if (ImGui::Selectable("Cosine")) {
			comboText = "Cosine";
			desorptionType = cosine;
		}
		if (ImGui::Selectable("Cosine^N")) {
			comboText = "Cosine^N";
			desorptionType = cosineN;
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (desorptionType == cosineN) {
		ImGui::SameLine();
		ImGui::SetNextItemWidth(txtW * 6);
		ImGui::InputText("###exponentIn", &exponentIn);
	}

	ImGui::End();
}

void ImOutgassingMap::ExplodeButtonPress()
{
}

void ImOutgassingMap::AutosizeButtonPress()
{
}

void ImOutgassingMap::PasteButtonPress()
{
}

void ImOutgassingMap::DrawTable()
{
}

void ImOutgassingMap::UpdateOnFacetChange(const std::vector<size_t>& selectedFacets)
{
	if (selectedFacets.size() > 0) {
		selFacetId = selectedFacets[0];
		name = "Outgassing map for Facet #" + std::to_string(selectedFacets[0] + 1);
	}
	else {
		selFacetId = -1;
		name = "Outgassing map";
	}
}

void ImOutgassingMap::OnShow()
{
	UpdateOnFacetChange(interfGeom->GetSelectedFacets());
}
