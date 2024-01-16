#include "ImguiFacetMirrorProject.h"

void ImFacetMirrorProject::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 20));
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::Begin("Mirror/project selected facets", &drawn, ImGuiWindowFlags_NoResize);
	ImGui::End();
}
