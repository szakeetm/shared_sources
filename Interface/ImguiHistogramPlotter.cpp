
#include "imgui.h"
#include "imgui_internal.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "ImguiHistogramPlotter.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "implot.h"
#include "implot_internal.h"
#include "Helper/MathTools.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

void ImHistogramPlotter::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 105, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::SetWindowPos("Histogram Plotter", ImVec2((3 + 40) * txtW, 4 * txtW), ImGuiCond_FirstUseEver);
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
#ifdef MOLFLOW
		if (ImGui::BeginTabItem("Flight time before absorption")) {
			plotTab = time;
			ImGui::EndTabItem();
		}
#endif
		ImGui::EndTabBar();
	}
	bool globalHist = ((plotTab == bounces && mApp->worker.model->sp.globalHistogramParams.recordBounce)
		|| (plotTab == distance && mApp->worker.model->sp.globalHistogramParams.recordDistance)
#ifdef MOLFLOW
		|| (plotTab == time && mApp->worker.model->sp.globalHistogramParams.recordTime)
#endif
		);
	if (prevPlotTab != plotTab) {
		// tab changed
		prevPlotTab = plotTab;
		UpdateSidebarMasterToggle();
	}
	ImGui::BeginChild("Sidebar", ImVec2(txtW*20,ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border);

	if (ImGui::TriState("###All", &aggregateState, mixedState)) {
		ApplyAggregateState();
	} ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	if (ImGui::Selectable("All", false, 0, ImVec2(ImGui::GetContentRegionAvail().x - txtW * 3, 0))) {
		for (int i = 0; i < histogrammedFacets[plotTab].size(); i++) {
			ShowFacet(histogrammedFacets[plotTab][i], true);
		}
	}
	ImGui::Separator();
	if (globalHist) {
		if (ImGui::Checkbox(("##Global"), (bool*)&(globalDrawToggle[plotTab]))) {
			if (globalDrawToggle[plotTab] == 0 && IsPlotted(-1)) RemovePlot(-1, plotTab);
			else if (globalDrawToggle[plotTab] == 1 && !IsPlotted(-1)) AddPlot(-1);
			UpdateSidebarMasterToggle();
		}
		ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Global");
	} 
	for (int i = 0; i < histogrammedFacets[plotTab].size(); i++) {
		std::string fName = ("Facet #" + std::to_string(histogrammedFacets[plotTab][i] + 1));
		if (ImGui::Checkbox(("##" + fName).c_str(), (bool*)&(histogramDrawToggle[plotTab][i]))) {
			if (histogramDrawToggle[plotTab][i] == 0 && IsPlotted(histogrammedFacets[plotTab][i])) RemovePlot(histogrammedFacets[plotTab][i], plotTab);
			else if (histogramDrawToggle[plotTab][i] == 1 && !IsPlotted(histogrammedFacets[plotTab][i])) AddPlot(histogrammedFacets[plotTab][i]);
			UpdateSidebarMasterToggle();
		} ImGui::SameLine();
		ImGui::AlignTextToFramePadding();
		if (ImGui::Selectable(fName)) {
			ShowFacet(i, ImGui::IsKeyDown(ImGuiKey_LeftShift));
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();
	DrawPlot();
	if(ImGui::Button("Histogram settings")) {
		settingsWindow.Toggle();
	} ImGui::SameLine();
	if (ImGui::Checkbox("Normalize", &normalize)) RefreshPlots();
	ImGui::SameLine();
	if (ImGui::Checkbox("Log Y", &logY)) RefreshPlots();
	ImGui::SameLine();
	if (ImGui::Checkbox("Log X", &logX)) RefreshPlots();
	if (settingsWindow.IsVisible()) {
		if (anchor) ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x - settingsWindow.width - txtW, ImGui::GetWindowPos().y));
	}
	ImGui::EndGroup();
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

	// load facet histograms from file
	
}

void ImHistogramPlotter::OnShow()
{
	RefreshFacetLists();
	settingsWindow.UpdateOnFacetChange();
	RefreshPlots();
}

void ImHistogramPlotter::DrawPlot()
{
	if(plotTab==bounces) xAxisName = "Number of bounces";
	if(plotTab==distance) xAxisName = "Distance [cm]";
#ifdef MOLFLOW
	if(plotTab==time) xAxisName = "Time [s]";
#endif
	ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 2);
	if (ImPlot::BeginPlot("##Histogram", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 5.7f * txtH))) {
		ImPlot::SetupAxis(ImAxis_X1, xAxisName.c_str(), ImPlotAxisFlags_AutoFit);
		ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_AutoFit);
		if (logX) ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
		if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
		for (auto& plot : data[plotTab]) {
			if (!plot.x || !plot.y || plot.x->size()==0 || plot.y->size()==0) continue;
			std::string name = plot.id == -1 ? "Global" : ("Facet #" + std::to_string(plot.id + 1));
			ImPlot::PlotScatter(name.c_str(), plot.x->data(), plot.y->data(), static_cast<int>(plot.x->size()));
			plot.color = ImPlot::GetLastItemColor();
		}
		bool isGlobal = (globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr);
		if (isGlobal) {
			ImPlot::PlotScatter("Global", globals[plotTab].x->data(), globals[plotTab].y->data(), static_cast<int>(globals[plotTab].x->size()));
			globals[plotTab].color = ImPlot::GetLastItemColor();
		}
		if (showValueOnHover) ImUtils::DrawValueOnHover(data[plotTab], isGlobal, globals[plotTab].x.get(), globals[plotTab].y.get());
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
}

void ImHistogramPlotter::RemovePlot(int idx, plotTabs tab)
{
	for (size_t i = 0; i < data[tab].size(); i++) {
		if (data[tab][i].id == idx) {
			data[tab].erase(data[tab].begin() + i);
			return;
		}
	}
	if (idx == -1) {
		globals[tab] = ImPlotData();
		return;
	}
	// a non-existent id was passed, fails silently here by design
}

void ImHistogramPlotter::AddPlot(int idx)
{
	if (idx < -1) return;
	if (IsPlotted(plotTab, idx)) return;
	ImPlotData newPlot;
	if (idx != -1) newPlot.id = idx;
	else newPlot.id = plotTab;
	
	newPlot.x = std::make_shared<std::vector<double>>();
	newPlot.y = std::make_shared<std::vector<double>>();

	if (idx != -1) {
		data[plotTab].push_back(newPlot);
		RefreshPlots();
		return;
	}
	globals[plotTab] = newPlot;
	RefreshPlots();
}

bool ImHistogramPlotter::IsPlotted(int idx)
{
	if (idx == -1) {
		return globals[plotTab].x != nullptr;
	}
	for (size_t i = 0; i < data[plotTab].size(); i++) {
		if (data[plotTab][i].id == idx) {
			data[plotTab].erase(data[plotTab].begin() + i);
			return true;
		}
	}
	return false;
}

void ImHistogramPlotter::RefreshPlots()
{
	auto lock = GetHitLock(mApp->worker.globalState.get(), 1000);
	double xMax = 1;

	for (auto& plot : data[plotTab]) { // facet histograms
		double xSpacing = 1;
		size_t nBins = 0;
		size_t facetId = plot.id;
		if (facetId < 0) continue;
		switch (plotTab) {
		case bounces:
			plot.y = std::make_shared<std::vector<double>>(interfGeom->GetFacet(facetId)->facetHistogramCache.nbHitsHistogram); // yaxis
			if (plot.y.get() == nullptr) break;
			xMax = static_cast<double>(interfGeom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceMax);
			xSpacing = static_cast<double>(interfGeom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceBinsize);
			nBins = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.GetBounceHistogramSize();
			break;
		case distance:
			plot.y = std::make_shared<std::vector<double>>(interfGeom->GetFacet(facetId)->facetHistogramCache.distanceHistogram); // yaxis
			if (plot.y.get() == nullptr) break;
			xMax = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.distanceMax;
			xSpacing = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.distanceBinsize;
			nBins = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.GetDistanceHistogramSize();
			break;
#ifdef MOLFLOW
		case time:
			plot.y = std::make_shared<std::vector<double>>(interfGeom->GetFacet(facetId)->facetHistogramCache.timeHistogram); // yaxis
			if (plot.y.get() == nullptr) break;
			xMax = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.timeMax;
			xSpacing = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.timeBinsize;
			nBins = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.GetTimeHistogramSize();
			break;
#endif
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
#ifdef MOLFLOW
	case time:
		*globals[plotTab].y.get() = mApp->worker.globalHistogramCache.timeHistogram;
		xMax = mApp->worker.model->sp.globalHistogramParams.timeMax;
		xSpacing = (double)mApp->worker.model->sp.globalHistogramParams.timeBinsize;
		nBins = mApp->worker.model->sp.globalHistogramParams.GetTimeHistogramSize();
		break;
#endif
	}
	// x axis
	globals[plotTab].x = std::make_shared<std::vector<double>>();
	if (!overrange) nBins--;
	for (size_t n = 0; n < nBins && (!limitPoints || n < maxDisplayed); n++) {
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
	std::vector<ImPlotData> preExportData = data[plotTab];
	ImPlotData preExportGlobal = globals[plotTab];
	bool preExportLimitPoints = limitPoints;
	size_t idx;
	if (!plottedOnly) {
		limitPoints = false;
		if (globals[plotTab].x.get()!=nullptr) {
			idx = -1;
			AddPlot(idx);
		}
		for (const auto& id: histogrammedFacets[plotTab]) {
			idx = static_cast<long>(id);
			AddPlot(idx);
		}
	}
	RefreshPlots();
	bool exportGlobal = globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr && globals[plotTab].x->size()!=0 && globals[plotTab].y->size() != 0;
	if (plottedOnly && !exportGlobal && data[plotTab].size() == 0) {
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
	out[out.size() - 1] = '\n';
	int n = std::max(exportGlobal ? static_cast<int>(globals[plotTab].x->size()) : 0, data[plotTab].size() == 0 ? 0 : static_cast<int>(data[plotTab][0].x->size()));
	for (int i = 0; i < n; ++i) {
		out.append(fmt::format("{}\t", (exportGlobal && i<globals[plotTab].x->size()) ? globals[plotTab].x->at(i) : data[plotTab][0].x->at(i)));// x value
		
		if (exportGlobal) {
			if (i < globals[plotTab].y->size()) {
				out.append(fmt::format("{}\t", globals[plotTab].y->at(i)));
			}
		}
		for (const auto& plot : data[plotTab]) {
			if (i < plot.y->size()) {
				out.append(fmt::format("{}\t", plot.y->at(i)));
			}
		}
		out[out.size() - 1] = '\n';
	}
	out.erase(out.end()-1);

	if (!toFile) SDL_SetClipboardText(out.c_str());
	else {
		std::string fileFilters = "txt,csv";
		std::string fn = NFD_SaveFile_Cpp(fileFilters, "");
		if (!fn.empty()) {
			FILE* f = fopen(fn.c_str(), "w");
			if (f == NULL) {
				ImIOWrappers::InfoPopup("Error", "Cannot open file\nFile: " + fn);
				return;
			}
			if (fn.find(".csv")!=std::string::npos) {
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
	data[plotTab] = preExportData;
	globals[plotTab] = preExportGlobal;
	limitPoints = preExportLimitPoints;
}

void ImHistogramPlotter::ShowFacet(int idx, bool add)
{
	if(!add) interfGeom->UnselectAll();
	try {
		interfGeom->GetFacet(idx)->selected = true;
	}
	catch (const std::exception& e) {
		ImIOWrappers::InfoPopup("Error", e.what());
	}
	LockWrapper lWrap(mApp->imguiRenderLock);
	interfGeom->UpdateSelection();
	mApp->UpdateFacetParams(true);
	mApp->UpdateFacetlistSelected();
}

void ImHistogramPlotter::ApplyAggregateState()
{
	bool globalHist = ((plotTab == bounces && mApp->worker.model->sp.globalHistogramParams.recordBounce)
		|| (plotTab == distance && mApp->worker.model->sp.globalHistogramParams.recordDistance)
#ifdef MOLFLOW
		|| (plotTab == time && mApp->worker.model->sp.globalHistogramParams.recordTime)
#endif
		);
	mixedState = false;
	globalDrawToggle[plotTab] = aggregateState;
	if ((globalDrawToggle[plotTab] == 0 && IsPlotted(-1)) || !globalHist) RemovePlot(-1, plotTab);
	else if (globalDrawToggle[plotTab] == 1 && globalHist && !IsPlotted(-1)) AddPlot(-1);
	for (int i = 0; i < histogrammedFacets[plotTab].size(); i++) {
		histogramDrawToggle[plotTab][i] = aggregateState;
		if (histogramDrawToggle[plotTab][i] == 1) AddPlot(histogrammedFacets[plotTab][i]);
		else if (histogramDrawToggle[plotTab][i] == 0) RemovePlot(histogrammedFacets[plotTab][i], plotTab);
	}
}

void ImHistogramPlotter::UpdateSidebarMasterToggle()
{
	histogramDrawToggle[plotTab].resize(histogrammedFacets[plotTab].size(), 0);
	mixedState = false;
	if (histogrammedFacets[plotTab].size() == 0 && (globals[plotTab].x.get() == nullptr || globals[plotTab].y.get() == nullptr)) {
		aggregateState = 0;
		return;
	}
	bool checkedFirst = false;
	for (int i = 0; i < histogrammedFacets[plotTab].size(); i++) {
		if (!checkedFirst) {
			aggregateState = histogramDrawToggle[plotTab][i];
			checkedFirst = true;
			continue;
		}
		if (aggregateState != histogramDrawToggle[plotTab][i]) {
			aggregateState = 2;
			mixedState = true;
			return;
		}
	}
	if (globalDrawToggle[plotTab] != aggregateState) {
		if (!checkedFirst) {
			aggregateState = globalDrawToggle[plotTab];
			checkedFirst = true;
		}
		else {
			aggregateState = 2;
			mixedState = true;
		}
	}
}

void ImHistogramPlotter::LoadHistogramSettings()
{
	// global settings
	settingsWindow.globalHistSet.recordBounce = mApp->worker.model->sp.globalHistogramParams.recordBounce;
	settingsWindow.globalRecordBounce = settingsWindow.globalHistSet.recordBounce;
	settingsWindow.globalHistSet.nbBounceMax = mApp->worker.model->sp.globalHistogramParams.nbBounceMax;
	settingsWindow.globalBouncesMaxInput = fmt::format("{}", settingsWindow.globalHistSet.nbBounceMax);
	settingsWindow.globalHistSet.nbBounceBinsize = mApp->worker.model->sp.globalHistogramParams.nbBounceBinsize;
	settingsWindow.globalBouncesBinSizeInput = fmt::format("{}", settingsWindow.globalHistSet.nbBounceBinsize);

	settingsWindow.globalHistSet.recordDistance = mApp->worker.model->sp.globalHistogramParams.recordDistance;
	settingsWindow.globalRecordDistance = settingsWindow.globalHistSet.recordDistance;
	settingsWindow.globalHistSet.distanceMax = mApp->worker.model->sp.globalHistogramParams.distanceMax;
	settingsWindow.globalDistanceMaxInput = fmt::format("{:.6g}", settingsWindow.globalHistSet.distanceMax);
	settingsWindow.globalHistSet.distanceBinsize = mApp->worker.model->sp.globalHistogramParams.distanceBinsize;
	settingsWindow.globalDistanceBinSizeInput = fmt::format("{:.6g}", settingsWindow.globalHistSet.distanceBinsize);
#ifdef MOLFLOW
	settingsWindow.globalHistSet.recordTime = mApp->worker.model->sp.globalHistogramParams.recordTime;
	settingsWindow.globalRecordTime = settingsWindow.globalHistSet.recordTime;
	settingsWindow.globalHistSet.timeMax = mApp->worker.model->sp.globalHistogramParams.timeMax;
	settingsWindow.globalTimeMaxInput = fmt::format("{:.6g}", settingsWindow.globalHistSet.timeMax);
	settingsWindow.globalHistSet.timeBinsize = mApp->worker.model->sp.globalHistogramParams.timeBinsize;
	settingsWindow.globalTimeBinSizeInput = fmt::format("{:.6g}", settingsWindow.globalHistSet.timeBinsize);
#endif
	// combo lists
	size_t n = interfGeom->GetNbFacet();
	for (size_t i = 0; i < n; i++) {
		const auto& facet = interfGeom->GetFacet(i);
		if (facet->facetHistogramCache.nbHitsHistogram.size() != 0 && !Contains(histogrammedFacets[bounces],i)) histogrammedFacets[bounces].push_back(i);
		if (facet->facetHistogramCache.distanceHistogram.size() != 0 && !Contains(histogrammedFacets[distance], i)) histogrammedFacets[distance].push_back(i);
#ifdef MOLFLOW
		if (facet->facetHistogramCache.timeHistogram.size() != 0 && !Contains(histogrammedFacets[time], i)) histogrammedFacets[time].push_back(i);
#endif
	}
	settingsWindow.CalculateMemoryEstimate_New(false);
	settingsWindow.CalculateMemoryEstimate_New(true);
	settingsWindow.CalculateMemoryEstimate_Current();
}

bool ImHistogramPlotter::IsPlotted(plotTabs tab, size_t facetId)
{
	for (const auto& plot : data[tab]) {
		if (plot.id == facetId) return true;
	}
	return false;
}

void ImHistogramPlotter::RefreshFacetLists()
{
	if (!mApp->worker.model->sp.globalHistogramParams.recordBounce) {
		globals[bounces] = ImPlotData();
	}
	histogrammedFacets[bounces].clear();

	if (!mApp->worker.model->sp.globalHistogramParams.recordDistance) {
		globals[distance] = ImPlotData();
	}
	histogrammedFacets[distance].clear();
#ifdef MOLFLOW
	if (!mApp->worker.model->sp.globalHistogramParams.recordTime) {
		globals[time] = ImPlotData();
	}
	histogrammedFacets[time].clear();
#endif
	std::vector<size_t> facets = interfGeom->GetAllFacetIndices();
	for (size_t idx : facets) { // get all facets which have histograms
		InterfaceFacet* f = interfGeom->GetFacet(idx);
		if (f->sh.facetHistogramParams.recordBounce) {
			histogrammedFacets[bounces].push_back(idx);
		}
		if (f->sh.facetHistogramParams.recordDistance) {
			histogrammedFacets[distance].push_back(idx);
		}
#ifdef MOLFLOW
		if (f->sh.facetHistogramParams.recordTime) {
			histogrammedFacets[time].push_back(idx);
		}
#endif
	}
	for (short tab = 0; tab < IM_HISTOGRAM_TABS; tab++) { // remove deleted facets from plot
		for (int i = static_cast<int>(data[tab].size() - 1); i >= 0; --i) {
			if (!Contains(histogrammedFacets[tab], data[tab][i].id)) {
				data[tab].erase(data[tab].begin() + i);
			}
		}
	}
	UpdateSidebarMasterToggle();
}

void ImHistogramPlotter::Reset()
{
	for (short tab = 0; tab < IM_HISTOGRAM_TABS; tab++) {
		data[tab].clear();
		if (globals[tab].x != nullptr) globals[tab].x.reset();
		if (globals[tab].y != nullptr) globals[tab].y.reset();
	}
	RefreshFacetLists();
}

void ImHistogramPlotter::UpdatePlotter()
{
	settingsWindow.UpdateOnFacetChange();
	RefreshFacetLists();
	RefreshPlots();
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
			//ImGui::Checkbox("Display overrange value", &overrange);
			ImGui::Checkbox("Anchor settings", &anchor);
			ImGui::EndMenu();
		}
		bool isGlobal = (globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr);
		bool isFacet =  data[plotTab].size() != 0 && data[plotTab].at(0).x.get() != nullptr && data[plotTab].at(0).y.get() != nullptr;
		long long bins = isGlobal ? globals[plotTab].y->size()!=0 : (isFacet ? data[plotTab].at(0).y->size() : 0);
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
	ImGui::SetNextWindowSize(ImVec2(width,34*txtH));
	ImGui::Begin("Histogram settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	float childHeight = (ImGui::GetContentRegionAvail().y - 1.5f*txtH) * 0.5f;

	if (ImGui::BeginChild("Global histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Global histogram");
		DrawSettingsGroup(globalHistSet, true, false);
	}
	ImGui::EndChild();
	if (ImGui::BeginChild("Facet histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Facet histogram");
		bool disabled = interfGeom->GetNbSelectedFacets() == 0;
		if (disabled) {
			ImGui::BeginDisabled();
		}
		
		// internal ImGui structure for data storage
		DrawSettingsGroup(facetHistSet, false, disabled);

		if (disabled) ImGui::EndDisabled();
	}
	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter(" Apply ");
	if (ImGui::Button("Apply")) Apply();
	ImGui::End();
}

void ImHistogramPlotter::ImHistogramSettings::UpdateOnFacetChange()
{
	EvaluateMixedState();
	facetHistSet = HistogramParams();
	CalculateMemoryEstimate_New(false);
}

bool ImHistogramPlotter::ImHistogramSettings::Apply()
{
	// wipe all global data
	//for (int i = 0; i < 3;i++) {
	//	parent->globals[i].x = std::make_shared<std::vector<double>>();
	//	parent->globals[i].y = std::make_shared<std::vector<double>>();
	//}

	// global
	parent->prevPlotTab=none;
	globalHistSet.recordBounce = globalRecordBounce;
	if (globalHistSet.recordBounce==1) {
		if (globalBouncesMaxInput != "...") {
			if (!Util::getNumber(&globalHistSet.nbBounceMax,globalBouncesMaxInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global bounce limit");
				return false;
			}
			else if (globalHistSet.nbBounceMax <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Global bounce limit must be a positive integer");
				return false;
			}
		}
		else {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global histogram parameters cannot be \"...\"");
			return false;
		}
		if (globalBouncesBinSizeInput != "...") {
			if (!Util::getNumber(&globalHistSet.nbBounceBinsize, globalBouncesBinSizeInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global bounce bin size");
				return false;
			}
			else if (globalHistSet.nbBounceBinsize <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Global bounce bin size must be a positive integer");
				return false;
			}
		}
		else {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global histogram parameters cannot be \"...\"");
			return false;
		}
	}
	globalHistSet.recordDistance = globalRecordDistance;
	if (globalHistSet.recordDistance==1) {
		if (globalDistanceMaxInput == "...") {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global histogram parameters cannot be \"...\"");
			return false;
		}
		else if (!Util::getNumber(&globalHistSet.distanceMax, globalDistanceMaxInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global distance limit");
			return false;
		}
		else if (globalHistSet.distanceMax <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global distance limit must be a positive scalar");
			return false;
		}
		if (globalDistanceBinSizeInput == "...") {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global histogram parameters cannot be \"...\"");
			return false;
		}
		else if (!Util::getNumber(&globalHistSet.distanceBinsize, globalDistanceBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global distance bin size");
			return false;
		}
		else if (globalHistSet.distanceBinsize <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global distance bin size must be a positive scalar");
			return false;
		}
	}
#if defined(MOLFLOW)
	globalHistSet.recordTime = globalRecordTime;
	if (globalHistSet.recordTime==1) {
		if (globalTimeMaxInput == "...") {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global histogram parameters cannot be \"...\"");
			return false;
		}
		else if (!Util::getNumber(&globalHistSet.timeMax, globalTimeMaxInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global time limit");
			return false;
		}
		else if (globalHistSet.timeMax <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global time limit must be a positive scalar");
			return false;
		}
		if (globalTimeBinSizeInput == "...") {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global histogram parameters cannot be \"...\"");
			return false;
		}
		else if (!Util::getNumber(&globalHistSet.timeBinsize, globalTimeBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global time bin size");
			return false;
		}
		else if (globalHistSet.timeBinsize<= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global time bin size must be a positive scalar");
			return false;
		}
#endif
	}
	if (interfGeom->GetNbSelectedFacets() != 0) {
		// facet
		if (facetRecordBounce != 2) facetHistSet.recordBounce = facetRecordBounce;
		if (facetHistSet.recordBounce == 1) {
			if (facetBouncesMaxInput == "...") {}
			else if (!Util::getNumber(&facetHistSet.nbBounceMax, facetBouncesMaxInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet bounce limit");
				return false;
			}
			else if (facetHistSet.nbBounceMax <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Facet bounce limit must be a positive integer");
				return false;
			}
			if (facetBouncesBinSizeInput == "...") {}
			else if (!Util::getNumber(&facetHistSet.nbBounceBinsize, facetBouncesBinSizeInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet bounce bin size");
				return false;
			}
			else if (facetHistSet.nbBounceBinsize <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Facet bounce bin size must be a positive integer");
				return false;
			}
		}
		if (facetRecordDistance != 2) facetHistSet.recordDistance = facetRecordDistance;
		if (facetRecordDistance==1) {
			if (facetDistanceMaxInput == "...") {}
			else if (!Util::getNumber(&facetHistSet.distanceMax, facetDistanceMaxInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet distance limit");
				return false;
			}
			else if (facetHistSet.distanceMax <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "facet distance limit must be a positive scalar");
				return false;
			}
			if (facetDistanceBinSizeInput == "...") {}
			else if (!Util::getNumber(&facetHistSet.distanceBinsize, facetDistanceBinSizeInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet distance bin size");
				return false;
			}
			else if (facetHistSet.distanceBinsize <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Facet distance bin size must be a positive scalar");
				return false;
			}
		}
	#if defined(MOLFLOW)
		if (facetRecordTime != 2) facetHistSet.recordTime = facetRecordTime;
		if (facetRecordTime==1) {
			if (facetTimeMaxInput == "...") {}
			else if (!Util::getNumber(&facetHistSet.timeMax, facetTimeMaxInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet time limit");
				return false;
			}
			else if (facetHistSet.timeMax <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Facet time limit must be a positive scalar");
				return false;
			}
			if (facetTimeBinSizeInput == "...") {}
			else if (!Util::getNumber(&facetHistSet.timeBinsize, facetTimeBinSizeInput)) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in facet time bin size");
				return false;
			}
			else if (facetHistSet.timeBinsize <= 0) {
				ImIOWrappers::InfoPopup("Histogram parameter error", "Facet time bin size must be a positive scalar");
				return false;
			}
	#endif
		}
	}


	// all entered values are valid, can proceed

	
	LockWrapper mLock(mApp->imguiRenderLock);
	if (!mApp->AskToReset()) return false; // early return if mApp gives problems
	
	
	if (globalRecordBounce != 2) mApp->worker.model->sp.globalHistogramParams.recordBounce = globalHistSet.recordBounce;
	if (globalBouncesMaxInput != "...") mApp->worker.model->sp.globalHistogramParams.nbBounceMax = globalHistSet.nbBounceMax;
	if (globalBouncesBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.nbBounceBinsize = globalHistSet.nbBounceBinsize;
	
	if (!mApp->worker.model->sp.globalHistogramParams.recordBounce) 
	{
		parent->globals[bounces].x.reset();
		parent->globals[bounces].y.reset();
	}

	if (globalRecordDistance != 2) mApp->worker.model->sp.globalHistogramParams.recordDistance = globalHistSet.recordDistance;
	if (globalDistanceMaxInput != "...") mApp->worker.model->sp.globalHistogramParams.distanceMax = globalHistSet.distanceMax;
	if (globalDistanceBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.distanceBinsize = globalHistSet.distanceBinsize;

	if (!mApp->worker.model->sp.globalHistogramParams.recordDistance) 
	{
		parent->globals[distance].x.reset();
		parent->globals[distance].y.reset();
	}
#ifdef MOLFLOW
	if (globalRecordTime != 2) mApp->worker.model->sp.globalHistogramParams.recordTime = globalHistSet.recordTime;
	if (globalTimeMaxInput != "...") mApp->worker.model->sp.globalHistogramParams.timeMax = globalHistSet.timeMax;
	if (globalTimeBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.timeBinsize = globalHistSet.timeBinsize;

	if (!mApp->worker.model->sp.globalHistogramParams.recordTime) 
	{
		parent->globals[time].x.reset();
		parent->globals[time].y.reset();
	}
#endif
	auto selectedFacets = interfGeom->GetSelectedFacets();
	for (const auto facetId : selectedFacets) { // for every facet
		/*
		for (int i = 0; i < IM_HISTOGRAM_TABS; i++) { // for every tab
			for (int j = 0; j < parent->data[i].size(); j++) // for every plotted graph 
				if (parent->data[i][j].id == facetId)
					parent->data[i][j] = ImPlotData();
		}
		*/
		InterfaceFacet* f = interfGeom->GetFacet(facetId);
		if (facetRecordBounce != 2) f->sh.facetHistogramParams.recordBounce = facetHistSet.recordBounce;
		if (facetBouncesMaxInput != "...") f->sh.facetHistogramParams.nbBounceMax = facetHistSet.nbBounceMax;
		if (facetBouncesBinSizeInput != "...") f->sh.facetHistogramParams.nbBounceBinsize = facetHistSet.nbBounceBinsize;
		if (facetRecordDistance != 2) f->sh.facetHistogramParams.recordDistance = facetHistSet.recordDistance;
		if (facetDistanceMaxInput != "...") f->sh.facetHistogramParams.distanceMax = facetHistSet.distanceMax;
		if (facetDistanceBinSizeInput != "...") f->sh.facetHistogramParams.distanceBinsize = facetHistSet.distanceBinsize;
#ifdef MOLFLOW
		if (facetRecordTime != 2) f->sh.facetHistogramParams.recordTime = facetHistSet.recordTime;
		if (facetTimeMaxInput != "...") f->sh.facetHistogramParams.timeMax = facetHistSet.timeMax;
		if (facetTimeBinSizeInput != "...") f->sh.facetHistogramParams.timeBinsize = facetHistSet.timeBinsize;
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
	parent->RefreshFacetLists();
	CalculateMemoryEstimate_Current();
	return true;
}

void ImHistogramPlotter::ImHistogramSettings::DrawSettingsGroup(HistogramParams& set, bool global, bool disabled)
{
	bool updateEstimate = false;
	if (ImGui::TriState("Record bounces until absorbtion", global ? &globalRecordBounce : &facetRecordBounce, global ? false : states.facetRecBounceMixed)) {
		if ((global ? globalRecordBounce : facetRecordBounce) != 2) {
			if (global) globalHistSet.recordBounce = globalRecordBounce;
			else facetHistSet.recordBounce = facetRecordBounce;
		}
		updateEstimate = true;
	}
	if (!disabled && (global ? globalRecordBounce : facetRecordBounce) != 1) ImGui::BeginDisabled();
	if (ImGui::InputTextRightSide("Max recorded no. of bounces:", global ? &globalBouncesMaxInput : &facetBouncesMaxInput, 0, txtW * 6)) {
		if ((global ? globalBouncesMaxInput : facetBouncesMaxInput) != "...") {
			if (global && Util::getNumber(&globalHistSet.nbBounceMax, globalBouncesMaxInput)) updateEstimate = true;
			else if (!global && Util::getNumber(&facetHistSet.nbBounceMax, facetBouncesMaxInput)) updateEstimate = true;
			else (global ? states.memNewGlobal : states.memNewFacet) = "[invalid textbox value(s)]";
		}
	}
	if (ImGui::InputTextRightSide("Bounces bin size:", global ? &globalBouncesBinSizeInput : &facetBouncesBinSizeInput, 0, txtW * 6)) {
		if ((global ? globalBouncesBinSizeInput : facetBouncesBinSizeInput) != "...") {
			if (global && Util::getNumber(&globalHistSet.nbBounceBinsize, globalBouncesBinSizeInput)) updateEstimate = true;
			else if (!global && Util::getNumber(&facetHistSet.nbBounceBinsize, facetBouncesBinSizeInput)) updateEstimate = true;
			else (global ? states.memNewGlobal : states.memNewFacet) = "[invalid textbox value(s)]";
		}
	}
	if (!disabled && (global ? globalRecordBounce : facetRecordBounce) != 1) ImGui::EndDisabled();

	if (ImGui::TriState("Record flight distance until absorption", global ? &globalRecordDistance : &facetRecordDistance, global ? false : states.facetRecDistanceMixed)) {
		if ((global ? globalRecordDistance : facetRecordDistance) != 2) {
			if (global) globalHistSet.recordDistance = globalRecordDistance;
			else facetHistSet.recordDistance = facetRecordDistance;
		}
		updateEstimate = true;
	}
	if (!disabled && (global ? globalRecordDistance : facetRecordDistance) != 1) ImGui::BeginDisabled();
	if (ImGui::InputTextRightSide("Max recorded flight distance (cm):", global ? &globalDistanceMaxInput : &facetDistanceMaxInput, 0, txtW * 6)) {
		if ((global ? globalDistanceMaxInput : facetDistanceMaxInput) != "...") {
			if (global && Util::getNumber(&globalHistSet.distanceMax, globalDistanceMaxInput)) updateEstimate = true;
			else if (!global && Util::getNumber(&facetHistSet.distanceMax, facetDistanceMaxInput)) updateEstimate = true;
			else (global ? states.memNewGlobal : states.memNewFacet) = "[invalid textbox value(s)]";
		}
	}
	if (ImGui::InputTextRightSide("Distance bin size (cm):", global ? &globalDistanceBinSizeInput : &facetDistanceBinSizeInput, 0, txtW * 6)) {
		if ((global ? globalDistanceBinSizeInput : facetDistanceBinSizeInput) != "...") {
			if (global && Util::getNumber(&globalHistSet.distanceBinsize, globalDistanceBinSizeInput)) updateEstimate = true;
			else if (!global && Util::getNumber(&facetHistSet.distanceBinsize, facetDistanceBinSizeInput)) updateEstimate = true;
			else (global ? states.memNewGlobal : states.memNewFacet) = "[invalid textbox value(s)]";
		}
	}
	if (!disabled && (global ? globalRecordDistance : facetRecordDistance) != 1) ImGui::EndDisabled();
#ifdef MOLFLOW
	if (ImGui::TriState("Record flight time until absorption", global ? &globalRecordTime : &facetRecordTime, global ? false : states.facetRecTimeMixed)) {
		if ((global ? globalRecordTime : facetRecordTime) != 2) {
			if (global) globalHistSet.recordTime = globalRecordTime;
			else facetHistSet.recordTime = facetRecordTime;
		}
		updateEstimate = true;
	}
	if (!disabled && (global ? globalRecordTime : facetRecordTime) != 1) ImGui::BeginDisabled();
	if (ImGui::InputTextRightSide("Max recorded flight time (s):", global ? &globalTimeMaxInput : &facetTimeMaxInput, 0, txtW * 6)) {
		if ((global ? globalTimeMaxInput : facetTimeMaxInput) != "...") {
			if (global && Util::getNumber(&globalHistSet.timeMax, globalTimeMaxInput)) updateEstimate = true;
			else if (!global && Util::getNumber(&facetHistSet.timeMax, facetTimeMaxInput)) updateEstimate = true;
			else (global ? states.memNewGlobal : states.memNewFacet) = "[invalid textbox value(s)]";
		}
	}
	if (ImGui::InputTextRightSide("Time bin size (s):", global ? &globalTimeBinSizeInput : &facetTimeBinSizeInput, 0, txtW * 6)) {
		if ((global ? globalTimeBinSizeInput : facetTimeBinSizeInput) != "...") {
			if (global && Util::getNumber(&globalHistSet.timeBinsize, globalTimeBinSizeInput)) updateEstimate = true;
			else if (!global && Util::getNumber(&facetHistSet.timeBinsize, facetTimeBinSizeInput)) updateEstimate = true;
			else (global ? states.memNewGlobal : states.memNewFacet) = "[invalid textbox value(s)]";
		}
	}
	if (!disabled && (global ? globalRecordTime : facetRecordTime) != 1) ImGui::EndDisabled();
#endif

	if (updateEstimate) {
		CalculateMemoryEstimate_New(global);
	}

	ImGui::Text(fmt::format("Current memory ({}): {}", global ? "global" : "all facets", FormatMemory(global ? states.memCurrentGlobal : states.memCurrentFacet)));
	ImGui::Text(fmt::format("After applying ({}): {}", global ? "global" : "sel facets", ((global ? states.memNewGlobal : states.memNewFacet))));
}

void ImHistogramPlotter::ImHistogramSettings::EvaluateMixedState()
{
	std::vector<size_t> selected = interfGeom->GetSelectedFacets();
	// load selection settings
	facetRecordBounce = 3;
	facetBouncesMaxInput = "";
	facetBouncesBinSizeInput = "";
	facetRecordDistance = 3;
	facetDistanceMaxInput = "";
	facetDistanceBinSizeInput = "";
#ifdef MOLFLOW
	facetRecordTime = 3;
	facetTimeMaxInput = "";
	facetTimeBinSizeInput = "";
#endif
	std::string tmp;
	short toggle;
	for (size_t facetIdx : selected) {
		InterfaceFacet* f = interfGeom->GetFacet(facetIdx);
		toggle = static_cast<bool>(f->sh.facetHistogramParams.recordBounce);
		if (facetRecordBounce == 3) facetRecordBounce = toggle;
		else if (facetRecordBounce != toggle) facetRecordBounce = 2;
		facetHistSet.nbBounceMax = f->sh.facetHistogramParams.nbBounceMax;
		tmp = fmt::format("{}", facetHistSet.nbBounceMax);
		if (facetBouncesMaxInput == "") {
			facetBouncesMaxInput = tmp; // first checked facet
		}
		else if (facetBouncesMaxInput != tmp) {
			facetBouncesMaxInput = "...";
		}
		facetHistSet.nbBounceBinsize = f->sh.facetHistogramParams.nbBounceBinsize;
		tmp = fmt::format("{}", facetHistSet.nbBounceBinsize);
		if (facetBouncesBinSizeInput == "") {
			facetBouncesBinSizeInput = tmp; // first checked facet
		}
		else if (facetBouncesBinSizeInput != tmp) {
			facetBouncesBinSizeInput = "...";
		}

		toggle = static_cast<bool>(f->sh.facetHistogramParams.recordDistance);
		if (facetRecordDistance == 3) facetRecordDistance = toggle;
		else if (facetRecordDistance != toggle) facetRecordDistance = 2;

		facetHistSet.distanceMax = f->sh.facetHistogramParams.distanceMax;
		tmp = fmt::format("{:.4g}", facetHistSet.distanceMax);
		if (facetDistanceMaxInput == "") {
			facetDistanceMaxInput = tmp; // first checked facet
		}
		else if (facetDistanceMaxInput != tmp) {
			facetDistanceMaxInput = "...";
		}
		facetHistSet.distanceBinsize = f->sh.facetHistogramParams.distanceBinsize;
		tmp = fmt::format("{:.4g}", facetHistSet.distanceBinsize);
		if (facetDistanceBinSizeInput == "") {
			facetDistanceBinSizeInput = tmp; // first checked facet
		}
		else if (facetDistanceBinSizeInput != tmp) {
			facetDistanceBinSizeInput = "...";
		}
#ifdef MOLFLOW
		toggle = static_cast<bool>(f->sh.facetHistogramParams.recordTime);
		if (facetRecordTime == 3) facetRecordTime = toggle;
		else if (facetRecordTime != toggle) facetRecordTime = 2;
		facetHistSet.timeMax = f->sh.facetHistogramParams.timeMax;
		tmp = fmt::format("{:.4g}", facetHistSet.timeMax);
		if (facetTimeMaxInput == "") {
			facetTimeMaxInput = tmp; // first checked facet
		}
		else if (facetTimeMaxInput != tmp) {
			facetTimeMaxInput = "...";
		}
		facetHistSet.timeBinsize = f->sh.facetHistogramParams.timeBinsize;
		tmp = fmt::format("{:.4g}", facetHistSet.timeBinsize);
		if (facetTimeBinSizeInput == "") {
			facetTimeBinSizeInput = tmp; // first checked facet
		}
		else if (facetTimeBinSizeInput != tmp) {
			facetTimeBinSizeInput = "...";
		}
#endif
	}
	if (selected.size() <= 1) { // one or none selected
		states.facetRecBounceMixed = false;
		states.facetRecDistanceMixed = false;
#ifdef MOLFLOW
		states.facetRecTimeMixed = false;
#endif
		return;
	}

	InterfaceFacet* f = interfGeom->GetFacet(selected[0]);
	bool facet1states[3];
	facet1states[0] = f->sh.facetHistogramParams.recordBounce;
	facet1states[1] = f->sh.facetHistogramParams.recordDistance;
#ifdef MOLFLOW
	facet1states[2] = f->sh.facetHistogramParams.recordTime;
#endif

	for (int i = 1; i < selected.size(); i++) {
		InterfaceFacet* f = interfGeom->GetFacet(selected[i]);
		if (facet1states[0] != f->sh.facetHistogramParams.recordBounce) {
			states.facetRecBounceMixed = true;
			facetRecordBounce = 2;
		}
		if (facet1states[1] != f->sh.facetHistogramParams.recordDistance) {
			states.facetRecDistanceMixed = true;
			facetRecordDistance = 2;
		}
#ifdef MOLFLOW
		if (facet1states[2] != f->sh.facetHistogramParams.recordTime) {
			states.facetRecTimeMixed = true;
			facetRecordTime = 2;
		}
		if (states.facetRecBounceMixed && states.facetRecDistanceMixed && states.facetRecTimeMixed) break;
#endif
		if (states.facetRecBounceMixed && states.facetRecDistanceMixed) break;
	}
}

void ImHistogramPlotter::ImHistogramSettings::CalculateMemoryEstimate_Current()
{
	states.memCurrentGlobal = mApp->worker.model->sp.globalHistogramParams.GetDataSize() * (mApp->worker.interfaceMomentCache.size() + 1);

	// calculate facet global hist mem est
	states.memCurrentFacet = 0;
	size_t nbFacet = interfGeom->GetNbFacet();
	for (size_t i = 0; i < nbFacet; i++) {
		states.memCurrentFacet += interfGeom->GetFacet(i)->sh.facetHistogramParams.GetDataSize();
	}
	states.memCurrentFacet *= (mApp->worker.interfaceMomentCache.size() + 1);
}

void ImHistogramPlotter::ImHistogramSettings::CalculateMemoryEstimate_New(bool global)
{
	if (global) {
		try {
			states.memNewGlobal = FormatMemory(globalHistSet.GetDataSize() * (mApp->worker.interfaceMomentCache.size() + 1));
		}
		catch (...) {
			states.memNewGlobal = "[invalid textbox value(s)]";
		}
	}
	else {
		try {
			bool mixed = false;
			if (facetRecordBounce == 2) mixed |= true;
			else if (facetRecordBounce == 1){
				mixed |= (facetBouncesMaxInput == "...");
				mixed |= (facetBouncesBinSizeInput == "...");
			}
			if (facetRecordDistance == 2) mixed |= true;
			else if (facetRecordDistance == 1) {
				mixed |= (facetDistanceMaxInput == "...");
				mixed |= (facetDistanceBinSizeInput == "...");
			}
#ifdef MOLFLOW
			if (facetRecordTime == 2) mixed |= true;
			else if (facetRecordTime == 1) {
				mixed |= (facetTimeMaxInput == "...");
				mixed |= (facetTimeBinSizeInput == "...");
			}
#endif
			if (mixed) {
				states.memNewFacet = "[mixed state]";
			}
			else {
				states.memNewFacet = FormatMemory(facetHistSet.GetDataSize() * interfGeom->GetNbSelectedFacets() * (mApp->worker.interfaceMomentCache.size() + 1));
			}
		}
		catch (...) {
			states.memNewFacet = "[invalid textbox value(s)]";
		}
	}
}
