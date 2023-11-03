#include "ImguiHistogramPlotter.h"
#include "imgui.h"

void ImHistogramPlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(3 * txtW, 4 * txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 15), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Histogram Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::End();
}
