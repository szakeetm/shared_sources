#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "ImguiHistogramPlotter.h"
#include "imgui.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"

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
	ImGui::Begin("Histogram Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
	DrawMenuBar();
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
	
	bool globalHist = ((plotTab == bounces && mApp->worker.model->sp.globalHistogramParams.recordBounce)
		|| (plotTab == distance && mApp->worker.model->sp.globalHistogramParams.recordDistance)
		|| (plotTab == time && mApp->worker.model->sp.globalHistogramParams.recordTime));

	if(!globalHist && comboSelection == -1) comboSelection = -2; // if global hist was selected but becomes disabled reset selection
	ImGui::SetNextItemWidth(txtW * 20);
	if (ImGui::BeginCombo("##HIST", comboSelection == -2 ? "" : (comboSelection == -1 ? "Global" : "Facet #" + std::to_string(comboSelection+1)))) {
		if (globalHist && ImGui::Selectable("Global")) comboSelection = -1;

		for (const auto facetId : comboOpts) {
			InterfaceFacet* f = interfGeom->GetFacet(facetId);
			bool showFacet = (plotTab == bounces && f->sh.facetHistogramParams.recordBounce)
				|| (plotTab == distance && f->sh.facetHistogramParams.recordDistance)
				|| (plotTab == time && f->sh.facetHistogramParams.recordTime);
			if (showFacet && ImGui::Selectable("Facet #" + std::to_string(facetId+1))) comboSelection = facetId;
		}

		ImGui::EndCombo();
	} ImGui::SameLine();
	if (ImGui::Button("<- Show Facet")) {
		interfGeom->UnselectAll();
		interfGeom->GetFacet(comboSelection)->selected = true;
	} ImGui::SameLine();
	if (ImGui::Button("Add")) {
		AddPlot();
	} ImGui::SameLine();
	if (ImGui::Button("Remove")) {
		RemovePlot();
	} ImGui::SameLine();
	if (ImGui::Button("Remove all")) {
		data[plotTab].clear();
		globals[plotTab] = ImPlotData();
	} ImGui::SameLine();
	if (ImGui::Checkbox("Normalize", &normalize)) RefreshPlots();
	ImGui::End();
	settingsWindow.Draw();
}

void ImHistogramPlotter::Init(Interface* mApp_)
{
	ImWindow::Init(mApp_);
	interfGeom = mApp->worker.GetGeometry();
	settingsWindow = ImHistogramSettings();
	settingsWindow.Init(mApp_);
	settingsWindow.parent = this;
}

void ImHistogramPlotter::DrawPlot()
{
	if(plotTab==bounces) xAxisName = "Number of bounces";
	if(plotTab==distance) xAxisName = "Distance [cm]";
	if(plotTab==time) xAxisName = "Time [s]";
	ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 2);
	if (ImPlot::BeginPlot("##Histogram", xAxisName.c_str(), 0, ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y - 6 * txtH), 0, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
		for (auto& plot : data[plotTab]) {
			if (!plot.x || !plot.y || plot.x->size()==0 || plot.y->size()==0) continue;
			std::string name = plot.id == -1 ? "Global" : ("Facet #" + std::to_string(plot.id + 1));
			ImPlot::PlotScatter(name.c_str(), plot.x->data(), plot.y->data(), plot.x->size());
			plot.color = ImPlot::GetLastItemColor();
		}
		if (globals[plotTab].x != nullptr && globals[plotTab].x->size() != 0) {
			ImPlot::PlotScatter("Global", globals[plotTab].x->data(), globals[plotTab].y->data(), globals[plotTab].x->size());
			globals[plotTab].color = ImPlot::GetLastItemColor();
		}
		bool isGlobal = (globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr);
		if (showValueOnHover) ImUtils::DrawValueOnHover(data[plotTab], isGlobal, globals[plotTab].x.get(), globals[plotTab].y.get());
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	RefreshPlots();
}

void ImHistogramPlotter::RemovePlot()
{
	for (size_t i = 0; i < data[plotTab].size(); i++) {
		if (data[plotTab][i].id == comboSelection) {
			data[plotTab].erase(data[plotTab].begin() + i);
			return;
		}
	}
	globals[plotTab] = ImPlotData();
}

void ImHistogramPlotter::AddPlot()
{
	ImPlotData newPlot;
	if (comboSelection != -1) newPlot.id = comboSelection;
	else newPlot.id = plotTab;
	
	newPlot.x = std::make_shared<std::vector<double>>();
	newPlot.y = std::make_shared<std::vector<double>>();

	if (comboSelection != -1) {
		data[plotTab].push_back(newPlot);
		return;
	}
	globals[plotTab] = newPlot;
	RefreshPlots();
}

void ImHistogramPlotter::RefreshPlots()
{
	GetHitLock(mApp->worker.globalState.get(), 1000);
	float xMax = 1;

	for (auto& plot : data[plotTab]) { // facet histograms
		double xSpacing = 1;
		size_t nBins = 0;
		size_t facetId = plot.id;
		switch (plotTab) {
		case bounces:
			*plot.y.get() = interfGeom->GetFacet(facetId)->facetHistogramCache.nbHitsHistogram; // yaxis
			xMax = (double)interfGeom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceMax;
			xSpacing = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceBinsize;
			nBins = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.GetBounceHistogramSize();
			break;
		case distance:
			*plot.y.get() = interfGeom->GetFacet(facetId)->facetHistogramCache.distanceHistogram; // yaxis
			xMax = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.distanceMax;
			xSpacing = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.distanceBinsize;
			nBins = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.GetDistanceHistogramSize();
			break;
		case time:
			*plot.y.get() = interfGeom->GetFacet(facetId)->facetHistogramCache.timeHistogram; // yaxis
			xMax = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.timeMax;
			xSpacing = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.timeBinsize;
			nBins = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.GetTimeHistogramSize();
			break;
		}
		
		// x axis
		plot.x = std::make_shared<std::vector<double>>();
		for (size_t n = 0; n < nBins && (limitPoints ? n < maxDisplayed : true); n++) {
			plot.x->push_back((double)n * xSpacing);
		}
		if (normalize) {
			double maxY = 0;
			for (unsigned int i = 0; i < plot.x->size(); i++) {
				maxY = std::max(maxY, plot.y->at(i));
			}
			if (maxY == 0) continue;
			double scaleY = 1 / maxY;
			for (unsigned int i = 0; i < plot.x->size(); i++) {
				plot.y->at(i) *= scaleY;
			}
		}
	}
	if (globals[plotTab].x.get() == nullptr || globals[plotTab].y.get() == nullptr) return;
	double xSpacing = 1;
	size_t nBins = 0;
	switch (plotTab) { // global histograms
	case bounces:
		*globals[plotTab].y.get() = mApp->worker.globalHistogramCache.nbHitsHistogram;
		xMax = (float)mApp->worker.model->sp.globalHistogramParams.nbBounceMax;
		xSpacing = (double)mApp->worker.model->sp.globalHistogramParams.nbBounceBinsize;
		nBins = mApp->worker.model->sp.globalHistogramParams.GetBounceHistogramSize();
		break;
	case distance:
		*globals[plotTab].y.get() = mApp->worker.globalHistogramCache.distanceHistogram;
		xMax = mApp->worker.model->sp.globalHistogramParams.distanceMax;
		xSpacing = (double)mApp->worker.model->sp.globalHistogramParams.distanceBinsize;
		nBins = mApp->worker.model->sp.globalHistogramParams.GetDistanceHistogramSize();
		break;
	case time:
		*globals[plotTab].y.get() = mApp->worker.globalHistogramCache.timeHistogram;
		xMax = mApp->worker.model->sp.globalHistogramParams.timeMax;
		xSpacing = (double)mApp->worker.model->sp.globalHistogramParams.timeBinsize;
		nBins = mApp->worker.model->sp.globalHistogramParams.GetTimeHistogramSize();
		break;
	}
	// x axis
	globals[plotTab].x = std::make_shared<std::vector<double>>();
	for (size_t n = 0; n < nBins-1 && (!limitPoints || n < maxDisplayed); n++) {
		globals[plotTab].x->push_back((double)n * xSpacing);
	}
	if (normalize) {
		double maxY = 0;
		for (unsigned int i = 0; i < globals[plotTab].x->size(); i++) {
			maxY = std::max(maxY, globals[plotTab].y->at(i));
		}
		if (maxY == 0) return;
		double scaleY = 1 / maxY;
		for (unsigned int i = 0; i < globals[plotTab].x->size(); i++) {
			globals[plotTab].y->at(i) *= scaleY;
		}
	}
}

void ImHistogramPlotter::Export(bool toFile, bool plottedOnly)
{
	if (!plottedOnly && limitPoints) { // get all available histogram data
		limitPoints = false;
		RefreshPlots();
		limitPoints = true;
	}
	bool exportGlobal = globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr;
	if (!exportGlobal && data[plotTab].size() == 0) {
		ImIOWrappers::InfoPopup("Error", "Nothing to export");
		return;
	}
	std::string out;
	out.append("X axis\t");
	// headers row
	if (exportGlobal) {
		out.append("Global\t");
	}
	for (const auto& plot : data[plotTab]) {
		out.append(fmt::format("Facet #{}\t", plot.id + 1));
	}
	out.append("\n");
	int n = exportGlobal ? globals[plotTab].x->size() : data[plotTab][0].x->size();
	for (int i = 0; i < n; ++i) {
		out.append(fmt::format("{}\t", exportGlobal ? globals[plotTab].x->at(i) : data[plotTab][0].x->at(i)));// x value
		
		if (exportGlobal) {
			out.append(fmt::format("{}\t", globals[plotTab].y->at(i)));
		}
		for (const auto& plot : data[plotTab]) {
			out.append(fmt::format("{}\t", plot.y->at(i)));
		}
		out.append("\n");
	}

	if (!toFile) SDL_SetClipboardText(out.c_str());
	else {
		std::string fileFilters = "txt";
		std::string fn = NFD_SaveFile_Cpp(fileFilters, "");
		if (!fn.empty()) {
			FILE* f = fopen(fn.c_str(), "w");
			if (f == NULL) {
				ImIOWrappers::InfoPopup("Error", "Cannot open file\nFile: " + fn);
				return;
			}
			fprintf(f, out.c_str());
			fclose(f);
		}
	}
}

void ImHistogramPlotter::DrawMenuBar()
{
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Export")) {
			if (ImGui::MenuItem("All to clipboard")) Export(false, false);
			if (ImGui::MenuItem("All to file")) Export(true, false);
			ImGui::Separator();
			if (ImGui::MenuItem("Plotted to clipboard")) Export(false, true);
			if (ImGui::MenuItem("Plotted to file")) Export(true, true);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			if(ImGui::Checkbox("Limit points", &limitPoints)) RefreshPlots();
			if (!limitPoints) ImGui::BeginDisabled();
			ImGui::Text("Limit"); ImGui::SameLine();
			if (ImGui::InputInt("##plotMax", &maxDisplayed, 100, 1000)) {
				if (maxDisplayed < 0) maxDisplayed = 0;
				RefreshPlots(); 
			}
			if (!limitPoints) ImGui::EndDisabled();
			ImGui::Checkbox("Display hovered value", &showValueOnHover);
			ImGui::EndMenu();
		}
		bool isGlobal = (globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr);
		bool isFacet =  data[plotTab].size() != 0;
		long long bins = isGlobal ? globals[plotTab].y->size() : isFacet ? data[plotTab].at(0).y->size() : 0;
		if ((isGlobal || isFacet) && bins>maxDisplayed && limitPoints) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), fmt::format("   Warning! Showing the first {} of {} values", maxDisplayed, bins).c_str());
		}

		ImGui::EndMenuBar();
	}
}

void ImHistogramPlotter::ImHistogramSettings::Draw()
{
	if (!drawn) return;
	width = 40 * txtW;
	ImGui::SetNextWindowSize(ImVec2(width,32*txtH));
	ImGui::Begin("Histogram settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

	float childHeight = (ImGui::GetContentRegionAvail().y - 1.5*txtH) * 0.5;

	if (ImGui::BeginChild("Global histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Global histogram");
		globalHistSet.amIDisabled = false;
		DrawSettingsGroup(globalHistSet);
	}
	ImGui::EndChild();
	if (ImGui::BeginChild("Facet histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Facet histogram");
		facetHistSet.amIDisabled = interfGeom->GetNbSelectedFacets() == 0;
		if (facetHistSet.amIDisabled) {
			ImGui::BeginDisabled();
		}
		
		// internal ImGui structure for data storage
		DrawSettingsGroup(facetHistSet);

		if (facetHistSet.amIDisabled) ImGui::EndDisabled();
	}
	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter(" Apply ");
	if (ImGui::Button("Apply")) Apply();
	ImGui::End();
}

bool ImHistogramPlotter::ImHistogramSettings::Apply()
{
	// wipe all data
	for (int i = 0; i < 3;i++) {
		parent->globals[i] = ImPlotData();
		for (int j = 0; j < parent->data[i].size(); j++) {
			parent->data[i][j] = ImPlotData();
		}
	}

	// global
	if (globalHistSet.globalRecBounce) {
		if (globalHistSet.maxRecNbBouncesInput != "...") {
			if (!Util::getNumber(&globalHistSet.nbBouncesMax,globalHistSet.maxRecNbBouncesInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global bounce limit");
				return false;
			}
			else if (globalHistSet.nbBouncesMax <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Global bounce limit must be a non-negative integer");
				return false;
			}
		}
		if (globalHistSet.bouncesBinSizeInput != "...") {
			if (!Util::getNumber(&globalHistSet.bouncesBinSize, globalHistSet.bouncesBinSizeInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global bounce bin size");
				return false;
			}
			else if (globalHistSet.bouncesBinSize <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Global bounce bin size must be a positive integer");
				return false;
			}
		}
	}
	if (globalHistSet.recFlightDist) {
		if (globalHistSet.maxFlightDistInput == "...") {}
		else if (!Util::getNumber(&globalHistSet.maxFlightDist, globalHistSet.maxFlightDistInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global distance limit");
			return false;
		}
		else if (globalHistSet.maxFlightDist <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global distance limit must be a non-negative scalar");
			return false;
		}
		if (globalHistSet.distBinSizeInput == "...") {}
		else if (!Util::getNumber(&globalHistSet.distBinSize, globalHistSet.distBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global distance bin size");
			return false;
		}
		else if (globalHistSet.distBinSize <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global distance bin size must be a positive scalar");
			return false;
		}
	}
#if defined(MOLFLOW)
	if (globalHistSet.recTime) {
		if (globalHistSet.maxFlightTimeInput == "...") {}
		else if (!Util::getNumber(&globalHistSet.maxFlightTime, globalHistSet.maxFlightTimeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global time limit");
			return false;
		}
		else if (globalHistSet.maxFlightTime <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global time limit must be a non-negative scalar");
			return false;
		}
		if (globalHistSet.timeBinSizeInput == "...") {}
		else if (!Util::getNumber(&globalHistSet.timeBinSize, globalHistSet.timeBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global time bin size");
			return false;
		}
		else if (globalHistSet.timeBinSize<= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global time bin size must be a positive scalar");
			return false;
		}
#endif
	}

	// facet
	if (facetHistSet.globalRecBounce) {
		if (facetHistSet.maxRecNbBouncesInput == "...") {}
		else if (!Util::getNumber(&facetHistSet.nbBouncesMax, facetHistSet.maxRecNbBouncesInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet bounce limit");
			return false;
		}
		else if (facetHistSet.nbBouncesMax <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Facet bounce limit must be a non-negative integer");
			return false;
		}
		if (facetHistSet.bouncesBinSizeInput == "...") {}
		else if (!Util::getNumber(&facetHistSet.bouncesBinSize, facetHistSet.bouncesBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet bounce bin size");
			return false;
		}
		else if (facetHistSet.bouncesBinSize <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Facet bounce bin size must be a positive integer");
			return false;
		}
	}
	if (facetHistSet.recFlightDist) {
		if (facetHistSet.maxFlightDistInput == "...") {}
		else if (!Util::getNumber(&facetHistSet.maxFlightDist, facetHistSet.maxFlightDistInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet distance limit");
			return false;
		}
		else if (facetHistSet.maxFlightDist <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "facet distance limit must be a non-negative scalar");
			return false;
		}
		if (facetHistSet.distBinSizeInput == "...") {}
		else if (!Util::getNumber(&facetHistSet.distBinSize, facetHistSet.distBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet distance bin size");
			return false;
		}
		else if (facetHistSet.distBinSize <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Facet distance bin size must be a positive scalar");
			return false;
		}
	}
#if defined(MOLFLOW)
	if (facetHistSet.recTime) {
		if (facetHistSet.maxFlightTimeInput == "...") {}
		else if (!Util::getNumber(&facetHistSet.maxFlightTime, facetHistSet.maxFlightTimeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet time limit");
			return false;
		}
		else if (facetHistSet.maxFlightTime <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Facet time limit must be a non-negative scalar");
			return false;
		}
		if (facetHistSet.timeBinSizeInput == "...") {}
		else if (!Util::getNumber(&facetHistSet.timeBinSize, facetHistSet.timeBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet time bin size");
			return false;
		}
		else if (facetHistSet.timeBinSize <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Facet time bin size must be a positive scalar");
			return false;
		}
#endif
	}

	// all entered values are valid, can proceed

	
	LockWrapper mLock(mApp->imguiRenderLock);
	if (!mApp->AskToReset()) return false; // early return if mApp gives problems
	mApp->worker.model->sp.globalHistogramParams.recordBounce = globalHistSet.globalRecBounce;
	if (globalHistSet.maxRecNbBouncesInput != "...") mApp->worker.model->sp.globalHistogramParams.nbBounceMax = globalHistSet.nbBouncesMax;
	if (globalHistSet.bouncesBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.nbBounceBinsize = globalHistSet.bouncesBinSize;
	mApp->worker.model->sp.globalHistogramParams.recordDistance = globalHistSet.recFlightDist;
	if (globalHistSet.maxFlightDistInput != "...") mApp->worker.model->sp.globalHistogramParams.distanceMax = globalHistSet.maxFlightDist;
	if (globalHistSet.distBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.distanceBinsize = globalHistSet.distBinSize;
#ifdef MOLFLOW
	mApp->worker.model->sp.globalHistogramParams.recordTime = globalHistSet.recTime;
	if (globalHistSet.maxFlightTimeInput != "...") mApp->worker.model->sp.globalHistogramParams.timeMax = globalHistSet.maxFlightTime;
	if (globalHistSet.timeBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.timeBinsize = globalHistSet.timeBinSize;
#endif
	auto selectedFacets = interfGeom->GetSelectedFacets();
	for (const auto facetId : selectedFacets) {
		parent->comboOpts.push_back(facetId);
		InterfaceFacet* f = interfGeom->GetFacet(facetId);
		f->sh.facetHistogramParams.recordBounce = globalHistSet.globalRecBounce;
		if (globalHistSet.maxRecNbBouncesInput != "...") f->sh.facetHistogramParams.nbBounceMax = globalHistSet.nbBouncesMax;
		if (globalHistSet.bouncesBinSizeInput != "...") f->sh.facetHistogramParams.nbBounceBinsize = globalHistSet.bouncesBinSize;
		f->sh.facetHistogramParams.recordDistance = globalHistSet.recFlightDist;
		if (globalHistSet.maxFlightDistInput != "...") f->sh.facetHistogramParams.distanceMax = globalHistSet.maxFlightDist;
		if (globalHistSet.distBinSizeInput != "...") f->sh.facetHistogramParams.distanceBinsize = globalHistSet.distBinSize;
#ifdef MOLFLOW
		f->sh.facetHistogramParams.recordTime = globalHistSet.recTime;
		if (globalHistSet.maxFlightTimeInput != "...") f->sh.facetHistogramParams.timeMax = globalHistSet.maxFlightTime;
		if (globalHistSet.timeBinSizeInput != "...") f->sh.facetHistogramParams.timeBinsize = globalHistSet.timeBinSize;
#endif
	}

	mApp->changedSinceSave = true;
	mApp->worker.needsReload = true; // to trigger realreload in update
	try {
		mApp->worker.Update(mApp->m_fTime); //To refresh histogram cache
	}
	catch (const std::exception& e) {
		ImIOWrappers::InfoPopup("Histogram Apply Error", e.what());
	}
	return true;
}

void ImHistogramPlotter::ImHistogramSettings::DrawSettingsGroup(histSet& set)
{
	ImGui::Checkbox("Record bounces until absorbtion", &set.globalRecBounce);
	if (!set.amIDisabled && !set.globalRecBounce) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded no. of bounces:", &set.maxRecNbBouncesInput,0,txtW*6);
	ImGui::InputTextRightSide("Bounces bin size:", &set.bouncesBinSizeInput,0,txtW*6);
	if (!set.amIDisabled && !set.globalRecBounce) ImGui::EndDisabled();

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

	// to the best of my understanding, this does not do anything in the Legacy code
	// if that is the case it can be removed
	ImGui::Text("Memory estimate of histogram: " + (set.showMemEst ? std::to_string(set.memEst) : ""));
}
