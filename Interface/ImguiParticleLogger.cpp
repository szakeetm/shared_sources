#include "ImguiParticleLogger.h"
#include "ImguiExtensions.h"

void ImParticleLogger::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowPos(ImVec2(5*txtW, 3*txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(50*txtW, 17*txtH));
	ImGui::Begin("Particle Logger", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::TextWrapped("This tool allows to record all test particles hitting a chosen facet.\nOnly one facet can be recorded at a time.\nThe recording must be exported, it is not saved with the file.");

	ImGui::BeginChild("##RecSet", ImVec2(0, 7 * txtH), true);
	{
		ImGui::TextDisabled("Recording settings");
		ImGui::Checkbox("Enable logging", &enableLogging);
		if (ImGui::BeginTable("##PL", 2)) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::InputTextRightSide("Facet number:", &facetNumInput);
			ImGui::InputTextRightSide("Max recorded:", &maxRecInput);
			ImGui::TableNextColumn();
			if (ImGui::Button("<- Get selected")) {}
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Mem");
		}
		ImGui::EndTable();
		ImGui::PlaceAtRegionCenter("Apply");
		if (ImGui::Button("Apply")) {}
	}
	ImGui::EndChild();
	ImGui::BeginChild("##Result", ImVec2(0, 4 * txtH), true);
	{
		ImGui::TextDisabled("Result");
		ImGui::Text("No rec");
		if (ImGui::Button("Copy to clipboard")) {}
		ImGui::SameLine();
		if (ImGui::Button("Export to CSV")) {}

	}
	ImGui::EndChild();

	ImGui::End();
}
