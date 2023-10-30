#include "ImguiTexturePlotter.h"
#include "Geometry_shared.h"
#include <vector>
#include <imgui.h>
#include "imgui_stdlib/imgui_stdlib.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

void ImTexturePlotter::Draw()
{
	if (!drawn) return;
	std::vector<size_t> facets = interfGeom->GetSelectedFacets();
	name = "Texture Plotter " + (facets.size() == 0 ? "[]" : "[Facet #" + std::to_string(facets[0]) + "]");
	ImGui::SetNextWindowSizeConstraints(ImVec2(65 * txtW, 15 * txtH), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin(name.c_str(), &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("##TPTab", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y - 5 * txtH),true);
	DrawTextureTable();
	ImGui::EndChild();
	if (ImGui::Button("Save")) {} ImGui::SameLine();
	if (ImGui::Button("FindMax")) {} ImGui::SameLine();

	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (31.5);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	ImGui::SetNextItemWidth(30 * txtW);
	if (ImGui::Combo("##View", &selIdx, u8"Cell Area (cm\u00B2)\0# of MC hits\0Impingment rate [1/m\u00B2/sec]]\0Particle density [1/m3]\0Pressure [mBar]\0Avg.speed estimate [m/s]\0Incident velocity vector[m/s]\0# of velocity vectors")) {
		switch (selIdx) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;
		}
	}
	if (ImGui::Button("Autosize")) {} ImGui::SameLine();
	ImGui::Checkbox("Autosize on every update (disable for smooth scrolling)", &autoSize);
	ImGui::SameLine();
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (8.25);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	if (ImGui::Button("Dismiss")) { Hide(); }

	ImGui::End();
}

void ImTexturePlotter::Init(Interface* mApp_)
{
	mApp = mApp_;
	interfGeom = mApp->worker.GetGeometry();
}

void ImTexturePlotter::DrawTextureTable()
{
}
