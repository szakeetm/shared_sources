#include "ImguiFacetCoordinates.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "ImguiWindow.h"
#include "ImguiExtensions.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ImFacetCoordinates::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 60, txtH * 20), ImVec2(txtW * 600, txtH*200));
	ImGui::Begin(name.c_str(), &drawn, ImGuiWindowFlags_NoSavedSettings);
	if (selFacet == nullptr) ImGui::BeginDisabled();
	DrawTable();
	ImGui::BeginChild("##FCC", ImVec2(0, ImGui::GetContentRegionAvail().y - 1.5 * txtH), true);
	ImGui::TextDisabled("Insert / Remove vertex");
	ImGui::Text("Vertex Id to insert:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("##VIID", &insertIdInput);
	if (ImGui::Button("Insert as last vertex")) {
		Insert(table.size());
	} ImGui::SameLine();
	if (ImGui::Button("Insert before sel. row")) {
		if (selRow == -1) ImIOWrappers::InfoPopup("Error", "No row selected");
		else {
			Insert(selRow);
			selRow--;
		}
	} ImGui::SameLine();
	if (ImGui::Button("Remove selected row")) {
		if (selRow == -1) ImIOWrappers::InfoPopup("Error", "No row selected");
		else {
			table.erase(table.begin() + selRow);
			selRow--;
		}
	}
	ImGui::EndChild();
	
	if (ImGui::Button("X")) {
		axis = X;
		mApp->imWnd->input.Open("Set all X coordinates to:", "New coordinate", [this](std::string v) { SetAllTo(v); });
	} ImGui::SameLine();

	if (ImGui::Button("Y")) {
		axis = Y;
		mApp->imWnd->input.Open("Set all Y coordinates to:", "New coordinate", [this](std::string v) { SetAllTo(v); });
	} ImGui::SameLine();

	if (ImGui::Button("Z")) {
		axis = Z;
		mApp->imWnd->input.Open("Set all Z coordinates to:", "New coordinate", [this](std::string v) { SetAllTo(v); });
	} ImGui::SameLine();
	
	ImGui::HelpMarker("Use the three axis buttons to bulk set coordinates to one value");
	ImGui::SameLine();
	float dummyWidth = ImGui::GetContentRegionAvail().x - 7 * txtW;
	ImGui::Dummy(ImVec2(dummyWidth, 0)); ImGui::SameLine();
	if (ImGui::Button("Apply")) {
		ApplyButtonPress();
	}

	if (selFacet == nullptr) ImGui::EndDisabled();
	ImGui::End();
}

void ImFacetCoordinates::OnShow()
{
	UpdateFromSelection();
}

void ImFacetCoordinates::DrawTable()
{
	ImGui::BeginChild("##FCTC", ImVec2(0, ImGui::GetContentRegionAvail().y - 6 * txtH));
	bool wasInput = false;

	int i = 0;
	if (ImGui::BeginTable("##FCT", 5, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders, ImVec2(ImGui::GetContentRegionAvail().x - (txtW/2), 0))) {
		ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, txtW*2);
		ImGui::TableSetupColumn("Vertex", ImGuiTableColumnFlags_WidthFixed, txtW * 5);
		ImGui::TableSetupColumn("X");
		ImGui::TableSetupColumn("Y");
		ImGui::TableSetupColumn("Z");
		ImGui::TableHeadersRow();
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH * 0.1));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // No padding between cells
		for (line& ln : table) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(fmt::format("{}", i + 1), selRow == i, selRow == i ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_SpanAllColumns)) {
				if (ValidateInputs(selRow)) {
					selRow = i;
				}
				wasInput = true;
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(fmt::format("{}",ln.vertexId+1));
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * 14);
			if (selRow != i) ImGui::Text(ln.coordInput[0]);
			else ImGui::InputText("###FCXI", &ln.coordInput[0]);
			ImGui::TableSetColumnIndex(3);
			ImGui::SetNextItemWidth(txtW * 14);
			if (selRow != i) ImGui::Text(ln.coordInput[1]);
			else ImGui::InputText("###FCYI", &ln.coordInput[1]);
			ImGui::TableSetColumnIndex(4);
			ImGui::SetNextItemWidth(txtW * 14);
			if (selRow != i) ImGui::Text(ln.coordInput[2]);
			else ImGui::InputText("###FCZI", &ln.coordInput[2]);
			i++;
		}
		ImGui::EndTable();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}
	ImGui::BeginChild("##clickCatcher", ImGui::GetContentRegionAvail());
	if (!wasInput && ImGui::IsAnyMouseDown() && ImGui::IsWindowHovered() && ValidateInputs(selRow)) selRow = -1;
	ImGui::EndChild();
	ImGui::EndChild();
}

void ImFacetCoordinates::ApplyButtonPress()
{
	for (int i = 0; i < table.size(); i++) {
		ValidateInputs(i);
	}
	if (table.size() < 3) {
		ImIOWrappers::InfoPopup("Not enough vertices", "A facet must have at least 3 vertices");
		return;
	}
	mApp->imWnd->popup.Open("Question", "Apply geometry changes?", { std::make_shared<ImIOWrappers::ImButtonFunc>("Ok",([this]() { Apply(); }),ImGuiKey_Enter, ImGuiKey_KeypadEnter),
		std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", SDL_SCANCODE_ESCAPE)});
}

void ImFacetCoordinates::Apply()
{
	mApp->changedSinceSave = true;

	//Change number of vertices
	selFacet->sh.nbIndex = (int)table.size();
	selFacet->indices.resize(selFacet->sh.nbIndex);
	selFacet->vertices2.resize(selFacet->sh.nbIndex);

	for (size_t i = 0; i < table.size(); i++) {
		interfGeom->MoveVertexTo(table[i].vertexId, table[i].coord.x, table[i].coord.y, table[i].coord.z);
		selFacet->indices[i] = table[i].vertexId;
	}
	interfGeom->Rebuild(); //Will recalculate facet parameters
	mApp->worker.MarkToReload();
}

void ImFacetCoordinates::Insert(int pos)
{
	if (!Util::getNumber(&insertID, insertIdInput)) {
		ImIOWrappers::InfoPopup("Error", "Could not parse vertex ID");
		return;
	}
	if (insertID-1 >= interfGeom->GetNbVertex()) {
		ImIOWrappers::InfoPopup("Error", fmt::format("Invalid vertex id Vertex {} doesn't exist.", insertID));
		return;
	}
	line newLine;
	newLine.vertexId = insertID-1;
	newLine.coord = *(mApp->worker.GetGeometry()->GetVertex(newLine.vertexId));
	newLine.coordInput[0] = fmt::format("{:.10g}", newLine.coord.x);
	newLine.coordInput[1] = fmt::format("{:.10g}", newLine.coord.y);
	newLine.coordInput[2] = fmt::format("{:.10g}", newLine.coord.z);
	table.insert(table.begin() + pos, newLine);
	selRow = pos;
}

void ImFacetCoordinates::UpdateFromSelection(const std::vector<size_t>& selectedFacets)
{
	table.clear();
	if (selectedFacets.size() == 0) {
		selFacet = nullptr;
		selFacetId = -1;
		name = "Facet coordinates###FCoords";
		return;
	}
	selFacetId = selectedFacets[0];
	selFacet = interfGeom->GetFacet(selFacetId);
	size_t nbIndex = selFacet->sh.nbIndex;
	for (size_t i = 0; i < nbIndex; i++) {
		line newLine;
		newLine.coord = *interfGeom->GetVertex(newLine.vertexId = selFacet->indices[i]); // copied from legacy
		newLine.coordInput[0] = fmt::format("{:.10g}", newLine.coord.x);
		newLine.coordInput[1] = fmt::format("{:.10g}", newLine.coord.y);
		newLine.coordInput[2] = fmt::format("{:.10g}", newLine.coord.z);
		table.push_back(newLine);
	}
	name = "Facet coordinates #" + std::to_string(selFacetId + 1) + "###FCoords";
	if (selRow >= table.size() || selRow<-1) selRow = -1;
}

void ImFacetCoordinates::UpdateFromSelection()
{
	UpdateFromSelection(interfGeom->GetSelectedFacets());
}

bool ImFacetCoordinates::ValidateInputs(int idx)
{
	if (idx >= table.size()) return true; // no such row, noting to check
	if (!(table[idx].vertexId >= 0 && table[idx].vertexId < interfGeom->GetNbVertex())) { //wrong coordinates at row
		ImIOWrappers::InfoPopup("Error", fmt::format("Invalid vertex id in row {}\n Vertex {} doesn't exist.", idx + 1, table[idx].vertexId + 1));
		return false;
	}
	if (!Util::getNumber(&table[idx].coord.x, table[idx].coordInput[0])) {
		ImIOWrappers::InfoPopup("Error", fmt::format("Invalid value in vertex {}, X coordinate", idx+1));
		return false;
	}
	if (!Util::getNumber(&table[idx].coord.y, table[idx].coordInput[1])) {
		ImIOWrappers::InfoPopup("Error", fmt::format("Invalid value in vertex {}, Y coordinate", idx+1));
		return false;
	}
	if (!Util::getNumber(&table[idx].coord.z, table[idx].coordInput[2])) {
		ImIOWrappers::InfoPopup("Error", fmt::format("Invalid value in vertex {}, Z coordinate", idx+1));
		return false;
	}
	return true;
}

void ImFacetCoordinates::SetAllTo(std::string val)
{
	float tmp = 0;
	if (!Util::getNumber(&tmp, val)) {
		ImIOWrappers::InfoPopup("Error", "Invalid input");
		return;
	}
	for (line& ln : table) {
		switch (axis) {
		case X:
			ln.coord.x = tmp;
			ln.coordInput[0] = fmt::format("{:.10g}", ln.coord.x);
			break;
		case Y:
			ln.coord.y = tmp;
			ln.coordInput[1] = fmt::format("{:.10g}", ln.coord.y);
			break;
		case Z:
			ln.coord.z = tmp;
			ln.coordInput[2] = fmt::format("{:.10g}", ln.coord.z);
			break;
		}
	}
}
