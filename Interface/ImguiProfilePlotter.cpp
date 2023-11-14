#include "ImguiProfilePlotter.h"
#include "Facet_shared.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"
#include "ProfileModes.h"
#include "Helper/StringHelper.h"
#include "Helper/MathTools.h"
#include "ImguiPopup.h"
#include "ImguiExtensions.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif
void ImProfilePlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(3 * txtW, 4 * txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 93, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Profile Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings);

	computeProfiles();
	DrawProfileGraph();

	ImGui::SetNextItemWidth(txtW * 30);
	size_t nFacets = interfGeom->GetNbFacet();
	if (ImGui::BeginCombo("##ProfilePlotterCombo", ((selectedProfile == -1 || f == 0) ? "Select [v] or type->" : ("F#" + std::to_string(selectedProfile + 1) + " " + molflowToUnicode(profileRecordModeDescriptions[(ProfileRecordModes)f->sh.profileType].second))))) {
		if (ImGui::Selectable("Select [v] or type->")) selectedProfile = -1;
		for (size_t i = 0; i < nFacets; i++) {
			if (!interfGeom->GetFacet(i)->sh.isProfile) continue;
			if (ImGui::Selectable("F#" + std::to_string(i + 1) + " " + molflowToUnicode(profileRecordModeDescriptions[(ProfileRecordModes)interfGeom->GetFacet(i)->sh.profileType].second) + "###profileCombo" + std::to_string(i), selectedProfile == i)) {
				selectedProfile = i;
				f = interfGeom->GetFacet(selectedProfile);
			}
		}
		ImGui::EndCombo();
	} ImGui::SameLine();
	
	ImGui::SetNextItemWidth(txtW * 15);
	if (selectedProfile != -1) ImGui::BeginDisabled();
	ImGui::InputText("##manualFacetSel", &manualFacetSel);
	if (selectedProfile != -1) ImGui::EndDisabled();
	ImGui::SameLine();
	if(ImGui::Button("Show Facet")) {
		ShowFacet();
	} ImGui::SameLine();
	if(ImGui::Button("Add Curve")) {
		AddCurve();
	} ImGui::SameLine();
	if(ImGui::Button("Remove Curve")) {
		RemoveCurve();
	} ImGui::SameLine();
	if(ImGui::Button("Remove all")) {
		data.clear();
		data.shrink_to_fit();
		drawManual = false;
	}
	ImGui::Text("Display as:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 15);
	ImGui::Combo("##View", &viewIdx, u8"Raw\0Pressure [mBar]\0Impingement rate [1/m\u00B2/sec]]\0Density [1/m3]\0Speed [m/s]\0Angle [deg]\0Normalize to 1");
	if (viewIdx == int(ProfileDisplayModes::Speed) || viewIdx == int(ProfileDisplayModes::Angle)) {
		ImGui::SameLine();
		ImGui::Checkbox("Surface->Volume conversion", &correctForGas);
	}


	ImGui::Checkbox("Colorblind mode", &colorBlind);
	ImGui::SameLine();
	ImGui::Text("Change linewidth:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	if (ImGui::InputFloat("##lineWidth", &lineWidth, 0.1, 1, "%.2f")) {
		if (lineWidth < 0.5) lineWidth = 0.5;
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Identify profiles in geometry", &identProfilesInGeom)) {
		FacetHiglighting(identProfilesInGeom);
	}

	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (18);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	if (ImGui::Button("Select plotted facets")) {
		interfGeom->UnselectAll();
		for (const auto& facet : data) {
			interfGeom->GetFacet(facet.id)->selected = true;
		}
		UpdateSelection();
	}

	ImGui::SetNextItemWidth(txtW * 40);
	ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
	if (ImGui::Button("-> Plot expression")) {
		drawManual = ImUtils::ParseExpression(expression, formula);
		if (manualPlot.x.get() == nullptr) manualPlot.x = std::make_shared<std::vector<double>>();
		if (manualPlot.y.get() == nullptr) manualPlot.y = std::make_shared<std::vector<double>>();

		ImUtils::ComputeManualExpression(drawManual, formula, *manualPlot.x.get(), *manualPlot.y.get(), profileSize);
	}
	ImGui::AlignTextToFramePadding();
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (11.25);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	ImGui::SameLine();
	ImGui::HelpMarker("Right-click plot to adjust fiting, scailing etc.\nScroll to zoom\nHold and drag to move\nHold right and drag for box select (auto-fit must be disabled first)");
	ImGui::SameLine();

	ImGui::End();
}

void ImProfilePlotter::Init(Interface* mApp_)
{
	ImWindow::Init(mApp_);
	interfGeom = mApp->worker.GetGeometry();
	ImPlot::GetStyle().AntiAliasedLines = true;
}

void ImProfilePlotter::DrawProfileGraph()
{
	lockYtoZero = data.size() == 0 && !drawManual;
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, lineWidth);
	if (ImPlot::BeginPlot("##ProfilePlot", "", 0, ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y - 7.5 * txtH),0, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
		for (auto& profile : data) {
			if (profile.x->size() == 0) continue;
			std::string name = "F#" + std::to_string(profile.id+1);
			ImPlot::PlotLine(name.c_str(), profile.x->data(), profile.y->data(),profile.x->size());
			profile.color = ImPlot::GetLastItemColor();
		}
		if (drawManual) ImPlot::PlotLine(formula.GetName().c_str(), manualPlot.x->data(), manualPlot.y->data(), manualPlot.x->size());
		DrawValueOnHover();
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	if (colorBlind) ImPlot::PopColormap();
	if (lockYtoZero) {
		ImPlotPlot& thisPlot = *ImPlot::GetPlot("##ProfilePlot");
		thisPlot.YAxis->SetMin(0, true);
		thisPlot.XAxis.SetMin(0, true);
		lockYtoZero = false;
	}
}

// repeated code with Convergence Plotter, todo: consolidate
void ImProfilePlotter::DrawValueOnHover()
{
	if (ImPlot::IsPlotHovered()) {
		ImPlotPoint mouse = ImPlot::GetPlotMousePos();
		if (data.size() != 0 || drawManual) {
			int entryIdx = -1;
			double minYDiff = ImPlot::PlotToPixels(ImPlotPoint(0, ImPlot::GetPlotLimits().Y.Size())).y*10;
			int plotIdx = -2;
			for (int i = 0; i < data.size(); i++) { // find which plot contains the value closest to cursor
				int entryIdxTmp = ImUtils::EntryIndexFromXAxisValue(mouse.x, data[i]);
				double dist = minYDiff;
				if (entryIdxTmp != -1) dist = abs(ImPlot::PlotToPixels(ImPlotPoint(0, mouse.y)).y - ImPlot::PlotToPixels(ImPlotPoint(0, data[i].y->at(entryIdxTmp))).y);
				if (entryIdxTmp != -1 && dist < minYDiff) {
					minYDiff = dist;
					plotIdx = i;
					entryIdx = entryIdxTmp;
				}
			}
			if (drawManual) {
				int entryIdxTmp = ImUtils::EntryIndexFromXAxisValue(mouse.x, manualPlot);
				double dist = minYDiff;
				if (entryIdxTmp != -1) dist = abs(ImPlot::PlotToPixels(ImPlotPoint(0, mouse.y)).y - ImPlot::PlotToPixels(ImPlotPoint(0, manualPlot.y->at(entryIdxTmp))).y);
				if (entryIdxTmp != -1 && dist < minYDiff) {
					minYDiff = dist;
					plotIdx = -1;
					entryIdx = entryIdxTmp;
				}
			}
			if (plotIdx == -1) {
				entryIdx = ImUtils::EntryIndexFromXAxisValue(mouse.x, manualPlot);
			}
			else if (plotIdx >= 0 && plotIdx < data.size()) {
				entryIdx = ImUtils::EntryIndexFromXAxisValue(mouse.x, data[plotIdx]);
			}
			if ( (plotIdx == -1 && entryIdx>=0 && entryIdx<manualPlot.x->size()) || (plotIdx>=0 && plotIdx<data.size() && entryIdx>=0 && entryIdx<data[plotIdx].x->size())) {
				double X = plotIdx == -1 ? manualPlot.x->at(entryIdx) : data[plotIdx].x->at(entryIdx);
				double Y = plotIdx == -1 ? manualPlot.y->at(entryIdx) : data[plotIdx].y->at(entryIdx);
				ImPlot::PushStyleColor(0, ImVec4(0, 0, 0, 1));
				ImPlot::PlotScatter("", &X, &Y, 1);
				ImPlot::PopStyleColor();

				std::string xVal, yVal;
				xVal = fmt::format("{:.4}", X);
				yVal = fmt::format("{:.4}", Y);

				ImVec2 tooltipPos = ImPlot::PlotToPixels(ImPlotPoint(X, Y));
				if (ImPlot::GetPlotPos().y + ImPlot::GetPlotSize().y - tooltipPos.y < 2 * txtH) tooltipPos.y -= 2.5 * txtH;
				if (ImPlot::GetPlotPos().x + ImPlot::GetPlotSize().x - tooltipPos.x < 20 * txtW) tooltipPos.x -= (std::max(xVal.length(), yVal.length())+2.5) * txtW;

				ImGui::SetNextWindowPos(tooltipPos);
				ImGui::BeginTooltipEx(0, 0);
				ImGui::Text("X=" + xVal + "\nY=" + yVal);
				ImGui::EndTooltip();
			}
		}
	}
}

void ImProfilePlotter::ShowFacet()
{
	interfGeom->UnselectAll();
	if (selectedProfile != -1) {
		interfGeom->GetFacet(selectedProfile)->selected = true;
	}
	else {
		std::vector<size_t> facetIds = manualFacetList();

		for (const auto facetId : facetIds) {
			interfGeom->GetFacet(facetId)->selected = true;
		}
	}
	UpdateSelection();
}

void ImProfilePlotter::AddCurve()
{
	if (selectedProfile != -1) {
		if (IsPlotted(selectedProfile)) {
			ImIOWrappers::InfoPopup("Error", "Already Plotted");
			return;
		}
		data.push_back({ selectedProfile, std::make_shared<std::vector<double>>(), std::make_shared<std::vector<double>>() });
		return;
	}

	std::vector<size_t> facetIds = manualFacetList();

	for (const auto& facetId : facetIds) {
		if (!IsPlotted(facetId)&&interfGeom->GetFacet(facetId)->sh.isProfile) {
			data.push_back({ facetId, std::make_shared<std::vector<double>>(), std::make_shared<std::vector<double>>() });
		}
	}
}

void ImProfilePlotter::RemoveCurve()
{
	if (data.size() == 0) return;
	if (selectedProfile == -1) {
		std::vector<size_t> facetIds = manualFacetList();
		long long i = data.size()-1; // has to be signed
		for (; i >= 0 && i<data.size(); i--) {
			for (const auto& facetId : facetIds) {
				if (data[i].id == facetId) {
					data.erase(data.begin() + i);
					break;
				}
			}
		}
		return;
	}
	for (size_t i = 0; i < data.size(); i++)
		if (data[i].id == selectedProfile) {
			data.erase(data.begin() + i);
			return;
		}
}

void ImProfilePlotter::computeProfiles()
{
	{
		LockWrapper lW(mApp->imguiRenderLock);
		if (!mApp->worker.ReloadIfNeeded()) return;
	}
	
	auto lock = GetHitLock(mApp->worker.globalState.get(), 10000);
	if (!lock) return;
	
	ProfileDisplayModes displayMode = static_cast<ProfileDisplayModes>(viewIdx); //Choosing by index is error-prone
	for (auto& plot : data) {

		if (plot.x->size() != profileSize) {
			plot.x->clear();
			for (size_t i = 0; i < profileSize; i++) plot.x->push_back(i);
		}
		plot.y->clear();
		InterfaceFacet* f = interfGeom->GetFacet(plot.id);
		const std::vector<ProfileSlice>& profile = mApp->worker.globalState->facetStates[plot.id].momentResults[mApp->worker.displayedMoment].profile;
		switch (viewIdx) {
		case static_cast<int>(ProfileDisplayModes::Raw):
			for (size_t j = 0; j < profileSize; j++)
				plot.y->push_back(profile[j].countEquiv);
			break;
		case static_cast<int>(ProfileDisplayModes::Pressure):
		{
			double scaleY = 1.0 / (f->GetArea() * 1E-4 / (double)PROFILE_SIZE) * mApp->worker.model->sp.gasMass / 1000 / 6E23 * 0.0100; //0.01: Pa->mbar
			scaleY *= mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
			for (size_t j = 0; j < profileSize; j++)
				plot.y->push_back(profile[j].sum_v_ort * scaleY);
		}
			break;
		case static_cast<int>(ProfileDisplayModes::ImpRate):
		{
			double scaleY = 1.0 / (f->GetArea() * 1E-4 / (double)PROFILE_SIZE);
			scaleY *= mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment);
			for (size_t j = 0; j < profileSize; j++)
				plot.y->push_back(profile[j].countEquiv * scaleY);
		}
			break;
		case static_cast<int>(ProfileDisplayModes::Density):
		{
			double scaleY = 1.0 / ((f->GetArea() * 1E-4) / (double)PROFILE_SIZE);
			scaleY *= mApp->worker.GetMoleculesPerTP(mApp->worker.displayedMoment) * f->DensityCorrection();

			for (int j = 0; j < PROFILE_SIZE; j++)
				plot.y->push_back(profile[j].sum_1_per_ort_velocity * scaleY);
		}
			break;
		case static_cast<int>(ProfileDisplayModes::Speed):
		{
			double sum = 0.0, val;
			double scaleX = f->sh.maxSpeed / (double)PROFILE_SIZE;
			std::vector<double> values;
			values.reserve(PROFILE_SIZE);
			for (int j = 0; j < PROFILE_SIZE; j++) {//count distribution sum
				if (!correctForGas)
					val = profile[j].countEquiv;
				else
					val = profile[j].countEquiv / (((double)j + 0.5) * scaleX); //fnbhit not needed, sum will take care of normalization
				sum += val;
				values.push_back(val);
			}
			plot.x->clear();
			for (int j = 0; j < PROFILE_SIZE; j++) {
				plot.x->push_back((double)j * scaleX);
				plot.y->push_back(values.at(j) / sum);
			}
		}
			break;
		case static_cast<int>(ProfileDisplayModes::Angle):
		{
			double sum = 0.0, val, scaleX;
			scaleX = 90.0 / (double)PROFILE_SIZE;
			std::vector<double> values;
			values.reserve(PROFILE_SIZE);
			for (int j = 0; j < PROFILE_SIZE; j++) {//count distribution sum
				if (!correctForGas)
					val = profile[j].countEquiv;
				else
					val = profile[j].countEquiv / sin(((double)j + 0.5) * PI / 2.0 / (double)PROFILE_SIZE); //fnbhit not needed, sum will take care of normalization
				sum += val;
				values.push_back(val);
			}
			for (int j = 0; j < PROFILE_SIZE; j++) {
				plot.x->push_back((double)j * scaleX);
				plot.y->push_back(values[j] / sum);
			}
		}
			break;
		case static_cast<int>(ProfileDisplayModes::NormalizeTo1):
		{
			double max = 1.0;

			for (int j = 0; j < PROFILE_SIZE; j++) {
				max = std::max(max, profile[j].countEquiv);
			}
			double scaleY = 1.0 / (double)max;

			for (int j = 0; j < PROFILE_SIZE; j++)
				plot.y->push_back(profile[j].countEquiv * scaleY);
		}
			break;
		}
	}
}

void ImProfilePlotter::UpdateSelection()
{
	LockWrapper lWrap(mApp->imguiRenderLock);
	interfGeom->UpdateSelection();
	mApp->UpdateFacetParams(true);
	mApp->UpdateFacetlistSelected();
}

// using GLColor type because it is deeply engrained
// in geometry shared code, could potentially modify
// geometry code to use ImVec4 in the future
void ImProfilePlotter::FacetHiglighting(bool toggle)
{
	if (!toggle) {
		interfGeom->SetPlottedFacets(std::map<int, GLColor>());
		UpdateSelection();
		return;
	}
	std::map<int, GLColor> colormap;
	for (const auto& plot : data) {
		std::pair<int, GLColor> pair;
		pair.first = plot.id;
		pair.second = GLColor(plot.color.x*255, plot.color.y*255, plot.color.z*255);
		colormap.emplace(pair);
	}
	interfGeom->SetPlottedFacets(colormap);
	UpdateSelection();
}

bool ImProfilePlotter::IsPlotted(size_t facetId)
{
	for (auto plot : data) {
		if (plot.id == facetId) return true;
	}
	return false;
}

std::vector<size_t> ImProfilePlotter::manualFacetList()
{
	std::vector<size_t> facetIds;
	try {
		splitFacetList(facetIds, manualFacetSel, interfGeom->GetNbFacet());
	}
	catch (const std::exception& e) {
		ImIOWrappers::InfoPopup("Error", e.what());
	}
	catch (...) {
		ImIOWrappers::InfoPopup("Error", "Unknown exception");
	}
	return facetIds;
}
