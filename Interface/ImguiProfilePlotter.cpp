#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiProfilePlotter.h"
#include "Facet_shared.h"
#include "implot.h"
#include "implot_internal.h"
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
	static bool wasDrawn = false;
	if (drawn != wasDrawn) {
		// visibility changed
		wasDrawn = drawn;
		FacetHiglighting(drawn ? identProfilesInGeom : false);
	}
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(3 * txtW, 4 * txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 82, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Profile Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);

	DrawMenuBar();

	ImGui::BeginChild("Sidebar", ImVec2(txtW * 20, ImGui::GetContentRegionAvail().y), true);
	if (ImGui::TriState("###All", &aggregateState, mixedState)) {
		ApplyAggregateState();
	} ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	if (ImGui::Selectable("All", false, 0, ImVec2(ImGui::GetContentRegionAvail().x - txtW * 3,0))) {
		for (int i = 0; i < profiledFacets.size(); i++) {
			ShowFacet(profiledFacets[i], true);
		}
	}
	ImGui::SameLine();
	ImGui::HelpMarker("Right-click plot or axis to adjust fiting\nScroll to zoom (with auto-fit off)\nHold and drag to move (auto-fit must be off)\nHold right and drag for box select (auto-fit must be off)\nToggle logarithmic Y axis in View menu\nClick on profile name to select corresponding facet in geometry, hold shift to add to selection");
	ImGui::Separator();
	size_t i = 0;
	for (size_t facetID : profiledFacets) {
		std::string fName = ("F#" + std::to_string(facetID + 1) + " " + molflowToUnicode(profileRecordModeDescriptions[(ProfileRecordModes)interfGeom->GetFacet(facetID)->sh.profileType].second));
		if (ImGui::Checkbox(("##"+fName).c_str(), (bool*)&(profileDrawToggle[i]))) {
			if (profileDrawToggle[i] == 0 && IsPlotted(facetID)) RemoveCurve(facetID);
			else if (profileDrawToggle[i] == 1 && !IsPlotted(facetID)) data.push_back(ImUtils::MakePlotData(facetID));
			UpdateSidebarMasterToggle();
			updateHilights = true;
		} ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		if (ImGui::Selectable(fName)) {
			ShowFacet(facetID, ImGui::IsKeyDown(ImGuiKey_LeftShift));
		}
		++i;
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();
	DrawProfileGraph();

	ImGui::AlignTextToFramePadding();
	ImGui::Text("Display as:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 27);
	if (ImGui::Combo("##View", &viewIdx, u8"Raw###0\0Pressure [mBar]###1\0Impingement rate [1/m\u00B2/sec]]###2\0Density [1/m3]###3\0Speed [m/s]###4\0Angle [deg]###5\0Normalize to 1###6\0")) {
		UpdatePlotter();
	}
	if (viewIdx == int(ProfileDisplayModes::Speed) || viewIdx == int(ProfileDisplayModes::Angle)) {
		ImGui::SameLine();
		if (ImGui::Checkbox("Surface->Volume conversion", &correctForGas)) {
			UpdatePlotter();
		}
	}
	ImGui::EndGroup();
	ImGui::End();
}

void ImProfilePlotter::Init(Interface* mApp_)
{
	ImWindow::Init(mApp_);
	interfGeom = mApp->worker.GetGeometry();
}

void ImProfilePlotter::LoadSettingsFromFile(bool log, std::vector<int> plotted)
{
	loading = true;
	data.clear();
	setLog = log;
	size_t nbFacets = interfGeom->GetNbFacet();
	for (int id : plotted) {
		if (id >= nbFacets || id<0) continue;
		InterfaceFacet* f = interfGeom->GetFacet(id);
		if (!f->sh.isProfile) continue;
		if (IsPlotted(id)) continue;
		data.push_back({ (size_t)id, std::make_shared<std::vector<double>>(), std::make_shared<std::vector<double>>() });
	}
	UpdateSidebarMasterToggle();
}

void ImProfilePlotter::UpdateProfiledFacetList()
{
	profiledFacets.clear();
	size_t nFacet = interfGeom->GetNbFacet();
	for (int i = 0; i < nFacet; i++) {
		if (!interfGeom->GetFacet(i)->sh.isProfile) continue;
		profiledFacets.push_back(i);
	}
	profileDrawToggle.resize(profiledFacets.size());
}

void ImProfilePlotter::OnShow() {
	Refresh();
}

void ImProfilePlotter::Refresh()
{
	UpdateProfiledFacetList();
	interfGeom = mApp->worker.GetGeometry();
	int nbFacet = static_cast<int>(interfGeom->GetNbFacet());
	for (int i = static_cast<int>(data.size()) - 1; i >= 0; i--) {
		if (data[i].id >= nbFacet) {
			RemoveCurve(static_cast<int>(data[i].id));
			continue;
		}
		InterfaceFacet* f = interfGeom->GetFacet(data[i].id);
		if (!f->sh.isProfile) RemoveCurve(static_cast<int>(data[i].id));
	}
	if (loading) loading = false;
	UpdatePlotter();
}

void ImProfilePlotter::UpdatePlotter()
{
	ComputeProfiles();
	UpdateSidebarMasterToggle();
}

void ImProfilePlotter::DrawProfileGraph()
{
	lockYtoZero = data.size() == 0 && !drawManual;
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, lineWidth);
	if (ImPlot::BeginPlot("##ProfilePlot", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 4.5f * txtH))) {
		ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_AutoFit);
		ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_AutoFit | (setLog ? ImPlotScale_Log10 : 0));
		if (setLog) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
		for (auto& profile : data) {
			std::string name = "F#" + std::to_string(profile.id+1);
			if (showDatapoints) ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::PlotLine(name.c_str(), profile.x->data(), profile.y->data(), static_cast<int>(profile.x->size()));
			profile.color = ImPlot::GetLastItemColor();
		}
		if (showDatapoints && drawManual) ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		if (drawManual) ImPlot::PlotLine(formula.GetExpression().c_str(), manualPlot.x->data(), manualPlot.y->data(), static_cast<int>(manualPlot.x->size()));
		if (showValueOnHover) ImUtils::DrawValueOnHover(data, drawManual, manualPlot.x.get(), manualPlot.y.get());
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	if (colorBlind) ImPlot::PopColormap();
	/*
	if (lockYtoZero) {
		ImPlotPlot& thisPlot = *ImPlot::GetPlot("##ProfilePlot");
		thisPlot.YAxis->SetMin(0, true);
		thisPlot.XAxis.SetMin(0, true);
		lockYtoZero = false;
	}
	*/
	if (updateHilights) {
		FacetHiglighting(identProfilesInGeom);
		updateHilights = false;
	}
}

void ImProfilePlotter::ShowFacet(size_t id, bool add)
{
	if(!add) interfGeom->UnselectAll();
	if (id != -1) {
		interfGeom->GetFacet(id)->selected = true;
	}
	UpdateSelection();
}

void ImProfilePlotter::RemoveCurve(size_t id)
{
	updateHilights = true;
	if (data.size() == 0) return;
	for (size_t i = 0; i < data.size(); i++) if (data[i].id == id) {
			data.erase(data.begin() + i);
			return;
		}
	UpdateSidebarMasterToggle();
}

void ImProfilePlotter::ComputeProfiles()
{
	{
		LockWrapper lW(mApp->imguiRenderLock);
		if (!mApp->worker.ReloadIfNeeded())
			return;
	}
	
	auto lock = GetHitLock(mApp->worker.globalState.get(), 10000);
	if (!lock) return;
	
	ProfileDisplayModes displayMode = static_cast<ProfileDisplayModes>(viewIdx); //Choosing by index is error-prone
	for (auto& plot : data) {
		if (plot.id > interfGeom->GetNbFacet()) {
			RemoveCurve(static_cast<int>(plot.id));
			return;
		}
		plot.y->clear();
		InterfaceFacet* f = interfGeom->GetFacet(plot.id);
		const std::vector<ProfileSlice>& profile = mApp->worker.globalState->facetStates[plot.id].momentResults[mApp->worker.displayedMoment].profile;
		bool isEmpty = true;
		for (const auto& value : profile) if (value.countEquiv != 0) isEmpty = false;
		if (isEmpty) {
			plot.x->clear();
			continue;
		}
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
		if (plot.x->size() != profileSize) {
			plot.x->clear();
			for (size_t i = 0; i < profileSize; i++) plot.x->push_back(static_cast<double>(i));
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
		pair.first = static_cast<int>(plot.id);
		pair.second = GLColor(static_cast<int>(plot.color.x*255.f), static_cast<int>(plot.color.y*255.f), static_cast<int>(plot.color.z*255.f));
		colormap.insert(pair);
	}
	interfGeom->SetPlottedFacets(colormap);
	UpdateSelection();
}

void ImProfilePlotter::DrawMenuBar()
{
	if (ImGui::BeginMenuBar()) {
		if(ImGui::BeginMenu("Export")) {
			if (ImGui::MenuItem("To clipboard")) Export();
			if (ImGui::MenuItem("To file")) Export(true);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			//ImGui::Checkbox("Colorblind mode", &colorBlind);
			ImGui::Checkbox("Log Y", &setLog);
			ImGui::Checkbox("Datapoints", &showDatapoints);
			ImGui::Text("Change linewidth:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(txtW * 12);
			if (ImGui::InputFloat("##lineWidth", &lineWidth, 0.1f, 1.f, "%.2f")) {
				if (lineWidth < 0.5f) lineWidth = 0.5f;
			}
			if (ImGui::Checkbox("Identify profiles in geometry", &identProfilesInGeom)) {
				FacetHiglighting(identProfilesInGeom);
			}
			ImGui::Checkbox("Display hovered value", &showValueOnHover);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Custom Plot")) {
			ImGui::SetNextItemWidth(txtW * 15);
			ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
			if (ImGui::Button("-> Plot expression") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
				drawManual = ImUtils::ParseExpression(expression, formula);
				if (manualPlot.x.get() == nullptr) manualPlot.x = std::make_shared<std::vector<double>>();
				if (manualPlot.y.get() == nullptr) manualPlot.y = std::make_shared<std::vector<double>>();
				manualPlot.x->clear();
				manualPlot.y->clear();

				if (drawManual) ImUtils::ComputeManualExpression(drawManual, formula, *manualPlot.x.get(), *manualPlot.y.get(), manualEnd, manualStart, manualStep);
				// no custom start, end, step bebause profiles have a constant size
			}
			ImGui::InputDoubleRightSide("Start X", &manualStart);
			ImGui::InputDoubleRightSide("End X", &manualEnd);
			ImGui::InputDoubleRightSide("Step:", &manualStep);
			if (manualEnd - manualStart < 0) ImGui::TextDisabled("Start X cannot be higer than End X");
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

bool ImProfilePlotter::Export(bool toFile)
{
	if (data.size() == 0) {
		ImIOWrappers::InfoPopup("Error", "Nothing to export");
		return false;
	}
	std::string out;
	// first row (headers)
	out.append("X axis\t");
	if (drawManual) out.append("manual\t");
	for (const auto& profile : data) {
		out.append("F#"+std::to_string(profile.id)+"\t");
	}
	out[out.size()-1]='\n';
	// rows
	for (int i = 0; i < data[0].x->size(); i++) {
		out.append(fmt::format("{}\t",data[0].x->at(i)));

		if (drawManual) {
			if (formula.GetNbVariable() != 0) {
				std::list<Variable>::iterator xvar = formula.GetVariableAt(0);
				xvar->value = data[0].x->at(i);
			}
			double yvar = formula.Evaluate();
			out.append(fmt::format("{}\t", yvar));
		}

		for (const auto& profile : data) {
			out.append(fmt::format("{}", profile.y->at(i))+"\t");
		}
		out[out.size() - 1] = '\n';
	}
	if(!toFile) SDL_SetClipboardText(out.c_str());
	else {
		std::string fileFilters = "txt,csv";
		std::string fn = NFD_SaveFile_Cpp(fileFilters, "");
		if (!fn.empty()) {
			FILE* f = fopen(fn.c_str(), "w");
			if (f == NULL) {
				ImIOWrappers::InfoPopup("Error", "Cannot open file\nFile: " + fn);
				return false;
			}
			if (fn.find(".csv") != std::string::npos) {
				size_t found = out.find('\t');
				while (found != std::string::npos) {
					out.replace(found, 1, ",");
					found = out.find('\t', found + 1);
				}
			}
			fprintf(f, out.c_str());
			fclose(f);
		}
	}
	return true;
}

bool ImProfilePlotter::IsPlotted(size_t facetId)
{
	for (auto plot : data) {
		if (plot.id == facetId) return true;
	}
	return false;
}

void ImProfilePlotter::ApplyAggregateState()
{
	mixedState = false;
	for (int i = 0; i < profileDrawToggle.size(); i++) {
		size_t facetID = profiledFacets[i];
		profileDrawToggle[i] = aggregateState;
		if (profileDrawToggle[i] == 0 && IsPlotted(facetID)) RemoveCurve(facetID);
		else if (profileDrawToggle[i] == 1 && !IsPlotted(facetID)) {
			data.push_back(ImUtils::MakePlotData(facetID));
		}
	}
	updateHilights = true;
}

void ImProfilePlotter::UpdateSidebarMasterToggle()
{
	mixedState = false;
	if (profileDrawToggle.size()==0) {
		aggregateState = 0;
		return;
	}
	bool checkedFirst = false;
	for (int i = 0; i < profileDrawToggle.size(); i++) {
		if (!checkedFirst) {
			aggregateState = profileDrawToggle[i];
			checkedFirst = true;
		}
		if (aggregateState != profileDrawToggle[i]) {
			aggregateState = 2;
			mixedState = true;
			return;
		}
	}
}
