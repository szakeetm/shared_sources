#include "ImguiVertexCoordinates.h"

void ImVertexCoordinates::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 20), ImGuiCond_FirstUseEver);
	ImGui::Begin("Vertex coordinates", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::End();
}
