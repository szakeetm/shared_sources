#include "ImguiMeasureForce.h"

void ImMeasureForce::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 75, txtH * 10), ImGuiCond_FirstUseEver);
	ImGui::Begin("Measure forces", &drawn, ImGuiWindowFlags_NoSavedSettings);



	ImGui::End();
}
