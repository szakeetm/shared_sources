#include "ImguiTextureScaling.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImTextureScailing::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(3*txtW, 3*txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(65*txtW,15*txtH),ImVec2(1000*txtW,100*txtH));
	
	ImGui::Begin("Texture Scailing", &drawn, ImGuiWindowFlags_NoSavedSettings);
	
	ImGui::BeginChild("Range", ImVec2(ImGui::GetContentRegionAvail().x - 15 * txtW, 6 * txtH), true);
	ImGui::TextDisabled("Texture Range");
	ImGui::BeginTable("##layoutHelper", 4, ImGuiTableFlags_SizingStretchProp);
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Min");
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Max");
		ImGui::TableNextColumn();
		ImGui::SetNextItemWidth(10 * txtW);
		ImGui::InputText("##minInput", &minInput);
		ImGui::SetNextItemWidth(10 * txtW);
		ImGui::InputText("##maxInput", &minInput);
		ImGui::TableNextColumn();
		ImGui::Checkbox("Autoscale", &autoscale);
		if(ImGui::BeginCombo("##includeCombo", "WIP")) {
			ImGui::EndCombo();
		}
		ImGui::TableNextColumn();
		ImGui::Checkbox("Use colors", &colors);
		ImGui::Checkbox("Logarithmic scale", &logScale);
	}
	ImGui::EndTable();
	if(ImGui::Button("Set to current")) {}
	ImGui::SameLine();
	if(ImGui::Button("Apply")) {}
	ImGui::SameLine();
	ImGui::Text("Swap: 0 bytes");
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("Current", ImVec2(ImGui::GetContentRegionAvail().x, 6 * txtH), true);
	ImGui::TextDisabled("Current");
	ImGui::Text("Min:");
	ImGui::Text("Max:");
	ImGui::EndChild();

	ImGui::BeginChild("Gradient", ImVec2(0, ImGui::GetContentRegionAvail().y-2*txtH), true);
	ImGui::TextDisabled("Gradient");
	ImGui::EndChild();
	ImGui::Text("Show"); ImGui::SameLine();
	if (ImGui::BeginCombo("##Show", "WIP")) {
		ImGui::EndCombo();
	}
	
	ImGui::End();
}
