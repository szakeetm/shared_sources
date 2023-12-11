#include "ImguiMeasureForce.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"

void ImMeasureForce::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 55, txtH * 9.5));
	ImGui::Begin("Measure forces", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::Checkbox("Enable force measurement (has performance impact)", &enableForceMeasurement);

	ImGui::BeginChild("###MFchild", ImVec2(0,ImGui::GetContentRegionAvail().y-txtH*1.5), true);
	
	ImGui::TextDisabled("Torque relative to...");

	if (ImGui::BeginTable("###MFT", 6, ImGuiTableFlags_SizingStretchProp)) {

		ImGui::TableNextRow();
		
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("mx0");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(txtW * 12);
		ImGui::InputText("###mx0", &mx0I);

		ImGui::TableSetColumnIndex(2);
		ImGui::Text("my0");
		ImGui::TableSetColumnIndex(3);
		ImGui::SetNextItemWidth(txtW * 12);
		ImGui::InputText("###my0", &my0I);

		ImGui::TableSetColumnIndex(4);
		ImGui::Text("mz0");
		ImGui::TableSetColumnIndex(5);
		ImGui::SetNextItemWidth(txtW * 12);
		ImGui::InputText("###mz0", &mz0I);

		ImGui::EndTable();
	}
	if (ImGui::Button("Selected vertex")) {
		SelectedVertexButtonPress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Center of selected facet")) {
		FacetCenterButtonPress();
	}

	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter(" Apply ");
	if (ImGui::Button("Apply")) {
		ApplyButtonPress();
	}
	ImGui::End();
}

void ImMeasureForce::SelectedVertexButtonPress()
{
}

void ImMeasureForce::FacetCenterButtonPress()
{
}

void ImMeasureForce::ApplyButtonPress()
{
}
