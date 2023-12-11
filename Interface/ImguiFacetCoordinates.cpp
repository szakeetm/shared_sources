#include "ImguiFacetCoordinates.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImFacetCoordinates::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 60, txtH * 20));
	ImGui::Begin("Facet coordinates", &drawn);
	DrawTable();
	ImGui::BeginChild("##FCC", ImVec2(0, ImGui::GetContentRegionAvail().y - 1.5 * txtH), true);
	ImGui::TextDisabled("Insert / Remove vertex");
	ImGui::Text("Vertex Id to insert:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("##VIID", &insertIdInput);
	if (ImGui::Button("Insert as last vertex")) {

	} ImGui::SameLine();
	if (ImGui::Button("Insert before sel. row")) {

	} ImGui::SameLine();
	if (ImGui::Button("Remove selected row")) {

	}
	ImGui::EndChild();
	
	if (ImGui::Button("X")) {

	} ImGui::SameLine();

	if (ImGui::Button("Y")) {

	} ImGui::SameLine();

	if (ImGui::Button("Z")) {

	} ImGui::SameLine();
	
	float dummyWidth = ImGui::GetContentRegionAvailWidth() - 6 * txtW;
	ImGui::Dummy(ImVec2(dummyWidth, 0)); ImGui::SameLine();
	if (ImGui::Button("Apply")) {
		ApplyButtonPress();
	}

	ImGui::End();
}

void ImFacetCoordinates::DrawTable()
{
	ImGui::BeginChild("##FCTC", ImVec2(0, ImGui::GetContentRegionAvail().y - 6 * txtH));

	if (ImGui::BeginTable("##FCT", 5)) {

		ImGui::EndTable();
	}

	ImGui::EndChild();
}

void ImFacetCoordinates::ApplyButtonPress()
{
}

void ImFacetCoordinates::Apply()
{
}

void ImFacetCoordinates::Insert(int pos)
{
}
