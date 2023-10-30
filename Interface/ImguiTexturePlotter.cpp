#include "ImguiTexturePlotter.h"
#include "Geometry_shared.h"
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
	if (facets.size() > 0) {
		if (facets[0] != selFacetId) {
			selFacetId = facets[0];
			name = "Texture Plotter [Facet #" + std::to_string(facets[0] + 1) + "]###TexturePlotter";
			selFacet = interfGeom->GetFacet(selFacetId);
			getData();
		}
	} else {
		selFacet = nullptr;
		selFacetId = -1;
		width = 0;
		height = 0;
		name = "Texture Plotter []###TexturePlotter";
	}
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
	ImGui::Combo("##View", &viewIdx, u8"Cell Area (cm\u00B2)\0# of MC hits\0Impingment rate [1/m\u00B2/sec]]\0Gas density [kg/m3]\0Particle density [1/m3]\0Pressure [mBar]\0Avg.speed estimate [m/s]\0Incident velocity vector[m/s]\0# of velocity vectors");
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
	if (width < 1 || height < 1) return;
	if (data.size() != width) getData();
	if (ImGui::BeginTable("##TPTable",width+1,ImGuiTableFlags_Borders)) {
		//headers
		ImGui::TableSetupColumn("v\\u", ImGuiTableColumnFlags_WidthFixed, txtW * 4); // corner
		for (int i = 0; i < width; ++i) ImGui::TableSetupColumn(std::to_string(i).c_str()); // indexes row
		ImGui::TableHeadersRow();

		for (int i = 0; i < height; i++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); // move to first column
			ImGui::Text(std::to_string(i+1).c_str());	//label column
			for (int j = 0; j < width; j++) {
				ImGui::TableSetColumnIndex(j+1);
				try {
					ImGui::Text(std::to_string(data[i][j]).c_str());
				}
				catch (...) {
					std::cout << "Error reading data" << std::endl;
				}
			}
		}
		ImGui::EndTable();
	}
}

void ImTexturePlotter::getData()
{
	for (int i = 0; i < data.size(); i++) std::vector<double>().swap(data[i]); // empty rows
	std::vector<std::vector<double>>().swap(data); // empty all
	size_t nbMoments = mApp->worker.interfaceMomentCache.size();
	maxValue = 0.0f;
	width = selFacet->sh.texWidth;
	height = selFacet->sh.texHeight;
	while (data.size() < height) data.push_back(std::vector<double>());

	switch (viewIdx) {
	case 0: // cell area
		break;
	case 1: // MC Hits
		break;
	case 2: // Impingment rate
		break;
	case 3: // particle density
		break;
	case 4: // Gas density
		break;
	case 5: {// Pressure
		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];

		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {

				PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::Pressure, moleculesPerTP, 1.0, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
				double p = val.value;

				if (p > maxValue) {
					maxValue = p;
					maxX = i; maxY = j;
				}

				data[j].push_back(p);
			}
		}
		break; }
	case 6: // Avg velocity
		break;
	case 7: // Gas velocity vector
		break;
	case 8: // Nb of velocity vectors
		break;
	}
}
