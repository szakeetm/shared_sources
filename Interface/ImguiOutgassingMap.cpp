#include "ImguiOutgassingMap.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiPopup.h"
#include "Interface.h"
#include "Helper/StringHelper.h"
#include "ImguiWindow.h"
#include <sstream>

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
	if (!facet || !facet->hasMesh) ImGui::BeginDisabled();
	if (ImGui::Button("Paste")) {
		PasteButtonPress();
	}
	if (!facet || !facet->hasMesh) ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::Text("Desorption type:");
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
	if (mApp->worker.GetGeometry()->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Exactly one facet has to be selected");
		return;
	}
	if (!facet->hasMesh) {
		ImIOWrappers::InfoPopup("Error", "Selected facet must have a mesh");
		return;
	}
	if (data.empty() || data[0].empty()) {
		return;
	}
	size_t w = data[0].size();
	size_t h = data.size();
	std::vector<double> values; // to be passed to ExplodeSelected()
	size_t rowN = 0, columnN = 0;
	for (auto& row : data) {
		columnN = 0;
		for (auto& cell : row) {
			if (facet->GetMeshArea(columnN + rowN * w)>0.0) { // skip outside
				double v = 0;
				if (!Util::getNumber(&v, cell)) { // verify correctness
					ImIOWrappers::InfoPopup("Error", fmt::format("Invalid outgassing number at Cell({},{})", rowN+1, columnN+1));
					return;
				}
				values.push_back(v); // add to vector
			}
			columnN++;
		}
		rowN++;
	}
	double desorbTypeN = 0;
	if (desorptionType == cosineN) {
		if (!Util::getNumber(&desorbTypeN, exponentIn)) {
			ImIOWrappers::InfoPopup("Error", "Exponent is not a number");
			return;
		}
		if (desorbTypeN <= 1) {
			ImIOWrappers::InfoPopup("Error", "Desorption type exponent must be greater than 1.0");
			return;
		}
	}
	std::function<void()> f = [this, desorbTypeN, values]() {
		LockWrapper lW(mApp->imguiRenderLock);
		if (mApp->AskToReset()) {
			mApp->changedSinceSave = true;
			try {
				mApp->worker.GetMolflowGeometry()->ExplodeSelected(true, (int)desorptionType, desorbTypeN, std::move(values.data()));
				mApp->UpdateModelParams();
				mApp->UpdateFacetParams(true);
				//worker->CalcTotalOutgassing();
				// Send to sub process
				mApp->worker.MarkToReload();
			}
			catch (...) {
				ImIOWrappers::InfoPopup("Error", "Error exploding facet");
			}
		}
		};
	mApp->imWnd->popup.Open("Confirm", "Explode selected facet?", 
		{	std::make_shared<ImIOWrappers::ImButtonFunc>("Ok",f,ImGuiKey_Enter,ImGuiKey_KeypadEnter),
			std::make_shared<ImIOWrappers::ImButtonInt>("Cancel",0,ImGuiKey_Escape)}
	);
}

void ImOutgassingMap::PasteButtonPress()
{
	if (!facet) return;
	if (selFacetId > interfGeom->GetNbFacet()) return;
	if (facet->cellPropertiesIds.empty()) return;
	if (!SDL_HasClipboardText()) return;

	size_t w = facet->sh.texWidth;
	size_t h = facet->sh.texHeight;
	
	std::string content(SDL_GetClipboardText());
	size_t rows = 0, columns = 0;

	// based on code by ChatGPT 3.5:
	std::istringstream iss(content);
	std::string line;
	bool exceeded = false;

	while (std::getline(iss, line)) {
		std::istringstream lineStream(line);
		std::string token;
		columns = 0;
		while (std::getline(lineStream, token, '\t')) {
			if (selection.column+columns < w) data[selection.row+rows][selection.column+columns]=token;
			else exceeded = true;
			columns++;
		}
		if (selection.row+rows >= h) {
			exceeded = true;
			break;
		}
		rows++;
	}
	if (exceeded) ImIOWrappers::InfoPopup("Warning", "Pasted data exceeded table size, some values were clipped (not included)");
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
	if (ImGui::BeginTable("###Outgass", facet->sh.texWidth+1, ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders)) {
		ImGui::TableSetupScrollFreeze(1, 1);
		// table setup
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH * 0.1));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // No padding between cells
		ImGui::TableSetupColumn(u8"v\u20D7\\u\u20D7", ImGuiTableColumnFlags_WidthFixed, txtW * 3); // corner
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
			ImU32 cell_bg_color = ImGui::GetColorU32(ImVec4(0.78f, 0.87f, 0.98f, 1.0f));
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
			for (auto& cell : row) {
				ImGui::TableSetColumnIndex(column);
				ImGui::SetNextItemWidth(txtW * 7);
				if (selection.row == rowN - 1 && selection.column == column - 1 && selection.active) {
					cell_bg_color = ImGui::GetColorU32(ImVec4(152.f/255.f, 186.f/255.f, 225.f/255.f, 1.0f));
					ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cell_bg_color);
					ImGui::InputText(fmt::format("###{}-{}", rowN, column), &cell);
				}
				else {
					if (ImGui::Selectable(fmt::format("{}###{}-{}", cell, rowN, column))) {
						selection.row = rowN - 1;
						selection.column = column - 1;
						selection.active = true;
					}
				}
				column++;
			}
			rowN++;
		}
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		if (selection.active && (ImGui::IsKeyDown(ImGuiKey_Enter) || ImGui::IsKeyDown(ImGuiKey_KeypadEnter))) {
			selection.active = false;
			selection.row = 0;
			selection.column = 0;
		}
		ImGui::EndTable();
	}
}

void ImOutgassingMap::UpdateData()
{
	data.clear();
	if (!facet) return;

	if (selFacetId > interfGeom->GetNbFacet()) return;

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
