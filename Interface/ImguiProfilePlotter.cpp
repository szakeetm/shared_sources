#include "ImguiProfilePlotter.h"
#include <imgui.h>
#include "Facet_shared.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot/implot.h"
#include "ProfileModes.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif
void ImProfilePlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(3 * txtW, 4 * txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 108, txtH * 15), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Profile Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings);

	DrawProfileGraph();
	
	ImGui::SetNextItemWidth(txtW * 30);
	static size_t selectedProfile = -1;
	InterfaceFacet* f = selectedProfile>-1 ? interfGeom->GetFacet(selectedProfile) : 0;
	if (ImGui::BeginCombo("##ProfilePlotterCombo", ((selectedProfile == -1) ? "Select [v] or type->" : ("F#" + std::to_string(selectedProfile) + " " + profileRecordModeDescriptions[(ProfileRecordModes)f->sh.profileType].second)))) {
		// for facet // for profile
		ImGui::EndCombo();
	} ImGui::SameLine();
	
	ImGui::SetNextItemWidth(txtW * 30);
	ImGui::InputText("##manualFacetSel", &manualFacetSel);
	ImGui::SameLine();
	if(ImGui::Button("Show Facet")) {} ImGui::SameLine();
	if(ImGui::Button("Add Curve")) {} ImGui::SameLine();
	if(ImGui::Button("Remove Curve")) {} ImGui::SameLine();
	if(ImGui::Button("Remove all")) {}

	static int viewIdx = 1;
	ImGui::Text("Display as:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 30);
	ImGui::Combo("##View", &viewIdx, u8"Raw\0Pressure [mBar]\0Impingement rate [1/m\u00B2/sec]]\0Density [1/m3]\0Speed [m/s]\0Angle [deg]\0Normalize to 1");
	ImGui::Checkbox("Colorblind mode", &colorBlind);
	ImGui::SameLine();
	ImGui::Text("Change linewidth:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	if (ImGui::InputFloat("##lineWidth", &lineWidth, 0.1, 1, "%.2f")) {
		if (lineWidth < 0.5) lineWidth = 0.5;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Identify profiles in geometry", &identProfilesInGeom);

	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (18);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	if (ImGui::Button("Select plotted facets")) {}

	ImGui::SetNextItemWidth(txtW * 40);
	ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
	if (ImGui::Button("-> Plot expression")) {
		//drawManual = PlotNewExpression();
		//computeManual();
	}
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Start X:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	ImGui::InputInt("##StartX", &startX); ImGui::SameLine();
	ImGui::Text("End X:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	ImGui::InputInt("##EndX", &endX); ImGui::SameLine();
	ImGui::Text("Step size:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	ImGui::InputDouble("##Step", &step, 1, 4, "%.2f");
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (8.25);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	if (ImGui::Button("Dismiss")) { Hide(); }

	ImGui::End();
}

void ImProfilePlotter::Init(Interface* mApp_)
{
	ImWindow::Init(mApp_);
	interfGeom = mApp->worker.GetGeometry();
}

void ImProfilePlotter::DrawProfileGraph()
{
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, lineWidth);
	ImPlot::SetNextPlotLimits(0, 1000, 0, 1000, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##ProfilePlot", "", 0, ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y - 8.5 * txtH))) {
		ImPlot::EndPlot();
	}
}