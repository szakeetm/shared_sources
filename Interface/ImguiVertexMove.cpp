#include "ImguiVertexMove.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImVertexMove::Draw() {
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 40, txtH * 20));
	ImGui::Begin("Move Vertex", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	if (ImGui::RadioButton("Absolute offset", mode == absOffset)) mode = absOffset;
	if (ImGui::RadioButton("Direction and distance", mode == directionDist)) mode = directionDist;
	std::string prefix = mode == absOffset ? "d" : "dir";
	ImGui::TextWithMargin(prefix + "X", 3 * txtW);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 15);
	ImGui::InputText("cm###xIn", &xIn);

	ImGui::TextWithMargin(prefix + "Y", 3 * txtW);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 15);
	ImGui::InputText("cm###yIn", &yIn);

	ImGui::TextWithMargin(prefix + "Z", 3 * txtW);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 15);
	ImGui::InputText("cm###zIn", &zIn);

	ImGui::BeginChild("###MVD", ImVec2(0, ImGui::GetContentRegionAvail().y-txtH*1.3),ImGuiChildFlags_Border);

	ImGui::Text("In direction");
	if (mode != directionDist)	ImGui::BeginDisabled();

	ImGui::Text("Distance");
	ImGui::SameLine();
	ImGui::InputText("cm###dIn", &dIn);

	if (mode != directionDist)	ImGui::EndDisabled();

	if (ImGui::Button("Facet normal")) FacetNormalButtonPress();

	if (ImGui::BeginTable("###MVlayoutHelper", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_BordersOuterH)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		ImGui::Text(baseMsg);
		if (ImGui::Button("Selected Vertex")) BaseSelVertButtonPress();
		if (ImGui::Button("Facet center")) BaseFacCentButtonPress();

		ImGui::TableSetColumnIndex(1);
		
		ImGui::Text(dirMsg);
		if (ImGui::Button("Selected Vertex")) DirSelVertButtonPress();
		if (ImGui::Button("Facet center")) DirFacCentButtonPress();

		ImGui::EndTable();
	}

	ImGui::EndChild();
	if (ImGui::Button("Move vertices")) ApplyButtonPress(false);
	ImGui::SameLine();
	if (ImGui::Button("Copy vertices")) ApplyButtonPress(true);


	ImGui::End();
}

void ImVertexMove::FacetNormalButtonPress()
{
}

void ImVertexMove::BaseSelVertButtonPress()
{
}

void ImVertexMove::BaseFacCentButtonPress()
{
}

void ImVertexMove::DirSelVertButtonPress()
{
}

void ImVertexMove::DirFacCentButtonPress()
{
}

void ImVertexMove::ApplyButtonPress(bool copy)
{
}
