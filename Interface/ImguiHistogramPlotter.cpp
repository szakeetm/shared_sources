#include "ImguiHistogramPlotter.h"
#include "imgui.h"
#include "implot/implot.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

void ImHistogramPlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(settingsWindow.width + (3 * txtW), 4 * txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 85, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
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
	if(ImGui::Button("<< Hist settings")) {
		ImGui::SetWindowPos("Histogram settings",ImVec2(ImGui::GetWindowPos().x-settingsWindow.width-txtW,ImGui::GetWindowPos().y));
		settingsWindow.Toggle();
	} ImGui::SameLine();
	
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
	settingsWindow.Draw();
}

void ImHistogramPlotter::Init(Interface* mApp_)
{
	__super::Init(mApp_);
	settingsWindow = ImHistagramSettings();
	settingsWindow.Init(mApp_);
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

void ImHistogramPlotter::ImHistagramSettings::Draw()
{
	if (!drawn) return;
	width = 40 * txtW;
	ImGui::SetNextWindowSize(ImVec2(width,32*txtH));
	ImGui::Begin("Histogram settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

	float childHeight = (ImGui::GetContentRegionAvail().y - 1.5*txtH) * 0.5;

	if (ImGui::BeginChild("Global histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Global histogram");
		globalHistSet.amIDisabled = false;
		Settings(globalHistSet);
		ImGui::EndChild();
	}
	if (ImGui::BeginChild("Facet histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Facet histogram");
		facetHistSet.amIDisabled = interfGeom->GetNbSelectedFacets() == 0;
		if (facetHistSet.amIDisabled) {
			ImGui::BeginDisabled();
		}
		
		// internal ImGui structure for data storage
		Settings(facetHistSet);

		if (facetHistSet.amIDisabled) ImGui::EndDisabled();
		ImGui::EndChild();
		ImGui::PlaceAtRegionCenter(" Apply ");
		if (ImGui::Button("Apply")) Apply();
	}
	ImGui::End();
}

bool ImHistogramPlotter::ImHistagramSettings::Apply()
{
	return false;
}

void ImHistogramPlotter::ImHistagramSettings::Settings(histSet& set)
{
	ImGui::Checkbox("Record bounces until absorbtion", &set.recBounce);
	if (!set.amIDisabled && !set.recBounce) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded no. of bounces:", &set.maxRecNbBouncesInput,0,txtW*6);
	ImGui::InputTextRightSide("Bounces bin size:", &set.bouncesBinSizeInput,0,txtW*6);
	if (!set.amIDisabled && !set.recBounce) ImGui::EndDisabled();

	ImGui::Checkbox("Record flight distance until absorption", &set.recFlightDist);
	if (!set.amIDisabled && !set.recFlightDist) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded flight distance (cm):", &set.maxFlightDistInput, 0, txtW * 6);
	ImGui::InputTextRightSide("Distance bin size (cm):", &set.distBinSizeInput, 0, txtW * 6);
	if (!set.amIDisabled && !set.recFlightDist) ImGui::EndDisabled();
	
	ImGui::Checkbox("Record flight time until absorption", &set.recTime);
	if (!set.amIDisabled && !set.recTime) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded flight time (s):", &set.maxFlightTimeInput, 0, txtW * 6);
	ImGui::InputTextRightSide("Time bin size (s):", &set.timeBinSizeInput, 0, txtW * 6);
	if (!set.amIDisabled && !set.recTime) ImGui::EndDisabled();

	ImGui::Text("Memory estimate of histogram: " + (set.showMemEst ? std::to_string(set.memEst) : ""));
}
