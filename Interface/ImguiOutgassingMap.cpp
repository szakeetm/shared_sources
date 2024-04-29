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

void ImOutgassingMap::PasteButtonPress()
{
}

void ImOutgassingMap::DrawTable()
{
	if (!facet)
	{
		ImGui::Text("No facet selected");
		return;
	}
	if (facet->cellPropertiesIds.empty()) {
		ImGui::Text("Selected facet does not have a mesh");
		return;
	}
	if (ImGui::BeginTable("###Outgass", facet->sh.texWidth+1, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg)) {
		// table setup
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH * 0.1));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // No padding between cells
		ImGui::TableSetupColumn("v\\u", ImGuiTableColumnFlags_WidthFixed, txtW * 3); // corner
		for (int i = 0; i < facet->sh.texWidth; ++i) {
			ImGui::TableSetupColumn(std::to_string(i + 1).c_str(), ImGuiTableColumnFlags_WidthFixed, txtW*7);
		}
		ImGui::TableHeadersRow();
		size_t rowN=1; // user facing so start from 1
		for (auto& row : data) {
			ImGui::TableNextRow();
			size_t column = 0;
			ImGui::TableSetColumnIndex(column++); // will increment column after the call
			ImGui::Text(fmt::format("{}",rowN));
			for (auto& cell : row) {
				ImGui::TableSetColumnIndex(column);
				ImGui::SetNextItemWidth(txtW * 7);
				ImGui::InputText(fmt::format("###{}-{}", rowN, column), &cell);
				column++;
			}
			rowN++;
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::EndTable();
	}
}

void ImOutgassingMap::UpdateData()
{
	data.clear();
	if (!facet) return;

	if (selFacetId > interfGeom->GetNbFacet()) throw std::exception("Invalid facet ID");

	if (facet->cellPropertiesIds.empty()) return;
	
	size_t w = facet->sh.texWidth;
	size_t h = facet->sh.texHeight;
	// data for drawing stored in row-column order
	data.resize(h); // resize for number of rows
	size_t j = 0;
	for (auto& row : data) {
		row.clear();
		row.reserve(w); // reserve for number of entries in a row (columns)
		for (size_t i = 0; i < w; i++) {
			if (facet->GetMeshArea(i + j * w) == 0.0) row.push_back("Outside");
			else row.push_back("0");
		}
		j++;
	}
}

void ImOutgassingMap::UpdateOnFacetChange(const std::vector<size_t>& selectedFacets)
{
	if (selectedFacets.size() > 0) {
		selFacetId = selectedFacets[0];
		name = "Outgassing map for Facet #" + std::to_string(selectedFacets[0] + 1);
		facet = interfGeom->GetFacet(selectedFacets[0]);
	}
	else {
		name = "Outgassing map";
		facet = nullptr;
	}
	UpdateData();
}

void ImOutgassingMap::OnShow()
{
	UpdateOnFacetChange(interfGeom->GetSelectedFacets());
}
