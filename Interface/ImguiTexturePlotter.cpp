#include "ImguiTexturePlotter.h"
#include "Geometry_shared.h"
#include <imgui.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/MathTools.h"

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

	dummyWidth = static_cast<float>(ImGui::GetContentRegionAvailWidth() - txtW * (31.5));
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	ImGui::SetNextItemWidth(30 * txtW);
	ImGui::Combo("##View", &viewIdx, u8"Cell Area (cm\u00B2)\0# of MC hits\0Impingment rate [1/m\u00B2/sec]]\0Gas density [kg/m3]\0Particle density [1/m3]\0Pressure [mBar]\0Avg.speed estimate [m/s]\0Incident velocity vector[m/s]\0# of velocity vectors");
	if (ImGui::Button("Autosize")) {} ImGui::SameLine();
	ImGui::Checkbox("Autosize on every update (disable for smooth scrolling)", &autoSize);
	ImGui::SameLine();
	ImGui::SameLine();
	dummyWidth = static_cast<float>(ImGui::GetContentRegionAvailWidth() - txtW * (8.25));
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
	static ImGuiIO& io = ImGui::GetIO();
	if (width < 1 || height < 1) return;
	if (data.size() != width) getData();
	if (ImGui::BeginTable("##TPTable", width+1,ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupScrollFreeze(1, 1);
		//headers
		ImGui::TableSetupColumn("v\\u", ImGuiTableColumnFlags_WidthFixed, txtW * 4); // corner
		for (int i = 0; i < width; ++i) {
			ImGui::TableSetupColumn(std::to_string(i + 1).c_str());
		}
		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		ImGui::TableSetColumnIndex(0);
		ImGui::Text("v\\u"); // todo unicode vector symbols
		for (int i = 0; i < width; ++i) {
			ImGui::TableSetColumnIndex(i + 1);
			const char* column_name = ImGui::TableGetColumnName(i+1); // Retrieve name passed to TableSetupColumn()
			ImGui::PushID(i+1);
			std::string id(column_name);
			if (ImGui::Selectable(id, false)) SelectColumn(i);
			//ImGui::SameLine();
			//ImGui::TableHeader(column_name);
			ImGui::PopID();
		}

		for (int i = 0; i < height; i++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); // move to first column
			if (ImGui::Selectable(std::to_string(i + 1), false)) {	//label row
				SelectRow(i);
			}
			for (int j = 0; j < width; j++) {
				ImGui::TableSetColumnIndex(j+1);
				bool isSelected = IsCellSelected(i,j);
				if(ImGui::Selectable(data[i][j]+"###" + std::to_string(i) + std::to_string(j), isSelected)) {
					selection.clear();
					selection.push_back(std::pair<int, int>(i, j));
				}
			}
		}
		ImGui::EndTable();
	}
}

void ImTexturePlotter::getData()
{
	if (!mApp->worker.ReloadIfNeeded()) return;
	auto lock = GetHitLock(mApp->worker.globalState.get(), 10000);
	if (!lock) return;
	
	for (int i = 0; i < data.size(); i++) std::vector<std::string>().swap(data[i]); // empty rows
	std::vector<std::vector<std::string>>().swap(data); // empty all
	size_t nbMoments = mApp->worker.interfaceMomentCache.size();
	maxValue = 0.0f;
	width = selFacet->sh.texWidth;
	height = selFacet->sh.texHeight;
	while (data.size() < height) data.push_back(std::vector<std::string>());


	switch (viewIdx) { // whole switch copied from legacy and slightly adjusted, a lot of repeted code, perhaps could be rewritten?
	case 0: {// Cell area
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				FacetMomentSnapshot* dummPointer = nullptr;
				double val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::CellArea, 1.0, 1.0, 1.0, (int)(i + j * width), *dummPointer).value;

				if (val > maxValue) {
					maxValue = val;
					maxX = i; maxY = j;
				}

				data[j].push_back(std::to_string(val));
			}
		}
		break; }
	case 1: {// MC Hits
		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];
		//TextureCell *texture = (TextureCell *)((BYTE *)buffer + (selFacet->sh.hitOffset + facetHitsSize + profSize + mApp->worker.displayedMoment*w*h * sizeof(TextureCell)));
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				//int tSize = selFacet->sp.texWidth*selFacet->sp.texHeight;

				PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::MCHits, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
				double realVal = val.value;
				if (realVal > maxValue) {
					maxValue = realVal;
					maxX = i; maxY = j;
				}
				data[j].push_back(std::to_string(realVal));
			}
		}
		break; }
	case 2: {// Impingement rate
		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];
		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				try {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::ImpingementRate, moleculesPerTP, 1.0, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
					double realVal = val.value;
					if (realVal > maxValue) {
						maxValue = realVal;
						maxX = i; maxY = j;
					}
					data[j].push_back(std::to_string(realVal));
				} catch (...) {
					data[j].push_back("Error");

				}
			}
		}

		break; }
	case 3: {// Particle density [1/m3]

		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];

		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		double densityCorrection = selFacet->DensityCorrection();

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::ParticleDensity, moleculesPerTP, densityCorrection, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
				double rho = val.value;

				if (rho > maxValue) {
					maxValue = rho;
					maxX = i; maxY = j;
				}
				data[j].push_back(std::to_string(rho));

			}
		}

		break; }
	case 4: {// Gas density [kg/m3]

		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];

		double moleculesPerTP = mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
		double densityCorrection = selFacet->DensityCorrection();

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::GasDensity, moleculesPerTP, densityCorrection, mApp->worker.model->sp.gasMass, (int)(i + j * width), facetSnapshot);
				double rho_mass = val.value;
				if (rho_mass > maxValue) {
					maxValue = rho_mass;
					maxX = i; maxY = j;
				}
				data[j].push_back(std::to_string(rho_mass));

			}
		}

		break; }
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

				data[j].push_back(std::to_string(p));
			}
		}
		break; }
	case 6: {// Average gas velocity [m/s]

		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::AvgGasVelocity, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
				double realVal = val.value;

				if (realVal > maxValue) {
					maxValue = realVal;
					maxX = i; maxY = j;
				}
				data[j].push_back(std::to_string(realVal));
			}
		}
		break; }
	case 7: {// Gas velocity vector

		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		size_t nbElem = selFacet->sh.texWidth * selFacet->sh.texHeight;
		size_t tSize = nbElem * sizeof(TextureCell);
		size_t dSize = nbElem * sizeof(DirectionCell);
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];
		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (selFacet->sh.countDirection) {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::GasVelocityVector, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
					Vector3d v_vect = val.vect;

					std::string out;
					out.append(std::to_string(v_vect.x));
					out.append(",");
					out.append(std::to_string(v_vect.y));
					out.append(",");
					out.append(std::to_string(v_vect.z));
					data[j].push_back(out);

					double length = v_vect.Norme();
					if (length > maxValue) {
						maxValue = length;
						maxX = i; maxY = j;
					}
				}
				else {
					data[j].push_back("Direction not recorded");
				}
			}
		}
		break; }
	case 8: {// Nb of velocity vectors

		size_t profSize = (selFacet->sh.isProfile) ? (PROFILE_SIZE * sizeof(ProfileSlice) * (1 + nbMoments)) : 0;
		size_t nbElem = selFacet->sh.texWidth * selFacet->sh.texHeight;
		size_t tSize = nbElem * sizeof(TextureCell);
		size_t dSize = nbElem * sizeof(DirectionCell);
		const auto& facetSnapshot = mApp->worker.globalState->facetStates[selFacetId].momentResults[mApp->worker.displayedMoment];

		for (size_t i = 0; i < width; i++) {
			for (size_t j = 0; j < height; j++) {
				if (selFacet->sh.countDirection) {
					PhysicalValue val = mApp->worker.GetGeometry()->GetPhysicalValue(selFacet, PhysicalMode::NbVelocityVectors, 1.0, 1.0, 1.0, (int)(i + j * width), facetSnapshot);
					size_t count = val.count;

					data[j].push_back(std::to_string(count));
					double countEq = (double)count;
					if (countEq > maxValue) {
						maxValue = countEq;
						maxX = i; maxY = j;
					}
				}
				else {
					data[j].push_back("Direction not recorded");
				}
			}
		}
		break; }
	}
}

bool ImTexturePlotter::IsCellSelected(size_t row, size_t col)
{
	bool out = false;

	for (const auto& pair : selection) {
		if (pair.first == row && pair.second == col) return true;
	}
	return out;;
}

void ImTexturePlotter::SelectRow(size_t row)
{
	selection.clear();
	for (size_t i = 0; i < width; i++) {
		selection.push_back(std::pair<int, int>(row,i));
	}
}

void ImTexturePlotter::SelectColumn(size_t col)
{
	selection.clear();
	for (size_t i = 0; i < width; i++) {
		selection.push_back(std::pair<int, int>(i, col));
	}
}

// todo
void ImTexturePlotter::DragSelect()
{
}
