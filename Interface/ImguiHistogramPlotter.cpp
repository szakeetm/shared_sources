#include "ImguiHistogramPlotter.h"
#include "imgui.h"
#include "implot/implot.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImHistogramPlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(3 * txtW, 4 * txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 85, txtH * 15), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Histogram Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings);

	if (ImGui::BeginTabBar("Histogram types")) {
		
		if (ImGui::BeginTabItem("Bounces before absorption")) {
			plotTab = bounces;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Flight distance before absorption")) {
			plotTab = distance;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Flight time before absorption")) {
			plotTab = time;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	DrawPlot();
	if(ImGui::Button("<< Hist settings")) {} ImGui::SameLine();
	
	ImGui::SetNextItemWidth(txtW * 20);
	if(ImGui::BeginCombo("##HIST","")) {
		ImGui::EndCombo();
	} ImGui::SameLine();
	if (ImGui::Button("<- Show Facet")) {} ImGui::SameLine();
	if (ImGui::Button("Add")) {} ImGui::SameLine();
	if (ImGui::Button("Remove")) {} ImGui::SameLine();
	if (ImGui::Button("Remove all")) {} ImGui::SameLine();
	ImGui::Checkbox("Normalize", &normalize);
	ImGui::End();
}

void ImHistogramPlotter::DrawPlot()
{
	if(plotTab==bounces) xAxisName = "Number of bounces";
	if(plotTab==distance) xAxisName = "Distance [cm]";
	if(plotTab==time) xAxisName = "Time [s]";
	if (ImPlot::BeginPlot("##Histogram", xAxisName.c_str(), 0, ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y - 4.5 * txtH))) {
		ImPlot::EndPlot();
	}
}
