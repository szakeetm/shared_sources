#include "ImguiBuildIntersection.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImBuildIntersect::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 40, txtH * 10));
	ImGui::Begin("Build Intersection", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::TextWrapped(
		"Select all facets that form part of the intersection. Then select vertices that you'd like to ensure are being kept. Facets without any selected vertex won't be altered"
	);
	if (ImGui::Button("Build Intersection")) BuildButtonPress();
	ImGui::SameLine();
	if (ImGui::Button("Undo")) UndoButtonPress();
	ImGui::Text(message);
	ImGui::End();
}

void ImBuildIntersect::BuildButtonPress()
{
}

void ImBuildIntersect::UndoButtonPress()
{
}
