#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "ImguiHistogramPlotter.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"
#include "Helper/MathTools.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

void ImHistogramPlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 85, txtH * 21), ImVec2(1000 * txtW, 100 * txtH));
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
		comboSelection = -2;
		if(comboOpts[plotTab].size()!=0) comboSelection = comboOpts[plotTab][0];
		if (globalHist) comboSelection = -1;
		prevPlotTab = plotTab;
	}
	DrawPlot();
	if(ImGui::Button("<< Hist settings")) {
		settingsWindow.Toggle();
	} ImGui::SameLine();
	
	if(!globalHist && comboSelection == -1) comboSelection = -2; // if global hist was selected but becomes disabled reset selection
	ImGui::SetNextItemWidth(txtW * 20);
	if (ImGui::BeginCombo("##HIST", comboSelection == -2 ? "" : (comboSelection == -1 ? "Global" : "Facet #" + std::to_string(comboSelection+1)))) {
		if (globalHist && ImGui::Selectable("Global")) comboSelection = -1;

		for (const auto facetId : comboOpts[plotTab]) {
			if (ImGui::Selectable("Facet #" + std::to_string(facetId+1))) comboSelection = facetId;
		}

		ImGui::EndCombo();
	} ImGui::SameLine();
	if (ImGui::Button("<- Show Facet")) {
		if (comboSelection >= 0) {
			interfGeom->UnselectAll();
			try {
				interfGeom->GetFacet(comboSelection)->selected = true;
			}
			catch (const std::exception& e) {
				ImIOWrappers::InfoPopup("Error", e.what());
			}
			LockWrapper lWrap(mApp->imguiRenderLock);
			interfGeom->UpdateSelection();
			mApp->UpdateFacetParams(true);
			mApp->UpdateFacetlistSelected();
		}
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
	if (ImGui::Checkbox("Log Y", &logY)) RefreshPlots();
	ImGui::SameLine();
	if (ImGui::Checkbox("Log X", &logX)) RefreshPlots();
	if (settingsWindow.IsVisible()) {
		if (anchor) ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x - settingsWindow.width - txtW, ImGui::GetWindowPos().y));
	}

	ImGui::End();
	settingsWindow.Draw();
	static std::vector<size_t> lastSel;
	std::vector<size_t> newSel = interfGeom->GetSelectedFacets();
	if (lastSel != newSel) {
		// load selection settings
		lastSel = newSel;
		settingsWindow.facetHistSet.recBounce = 3;
		settingsWindow.facetHistSet.maxRecNbBouncesInput = "";
		settingsWindow.facetHistSet.bouncesBinSizeInput = "";
		settingsWindow.facetHistSet.recFlightDist = 3;
		settingsWindow.facetHistSet.maxFlightDistInput = "";
		settingsWindow.facetHistSet.distBinSizeInput = "";
#ifdef MOLFLOW
		settingsWindow.facetHistSet.recTime = 3;
		settingsWindow.facetHistSet.maxFlightTimeInput = "";
		settingsWindow.facetHistSet.timeBinSizeInput = "";
#endif
		std::string tmp;
		bool toggle;
		for (size_t facetIdx : lastSel) {
			InterfaceFacet* f = interfGeom->GetFacet(facetIdx);
			toggle = f->sh.facetHistogramParams.recordBounce;
			if (settingsWindow.facetHistSet.recBounce == 3) settingsWindow.facetHistSet.recBounce = toggle;
			else if (settingsWindow.facetHistSet.recBounce != toggle) settingsWindow.facetHistSet.recBounce = 2;
			settingsWindow.facetHistSet.nbBouncesMax = f->sh.facetHistogramParams.nbBounceMax;
			tmp = fmt::format("{}", settingsWindow.facetHistSet.nbBouncesMax);
			if (settingsWindow.facetHistSet.maxRecNbBouncesInput == "") {
				settingsWindow.facetHistSet.maxRecNbBouncesInput = tmp; // first checked facet
			}
			else if (settingsWindow.facetHistSet.maxRecNbBouncesInput != tmp) {
				settingsWindow.facetHistSet.maxRecNbBouncesInput = "...";
			}
			settingsWindow.facetHistSet.bouncesBinSize = f->sh.facetHistogramParams.nbBounceBinsize;
			tmp = fmt::format("{}", settingsWindow.facetHistSet.bouncesBinSize);
			if (settingsWindow.facetHistSet.bouncesBinSizeInput == "") {
				settingsWindow.facetHistSet.bouncesBinSizeInput = tmp; // first checked facet
			}
			else if (settingsWindow.facetHistSet.bouncesBinSizeInput != tmp) {
				settingsWindow.facetHistSet.bouncesBinSizeInput = "...";
			}

			toggle = f->sh.facetHistogramParams.recordDistance;
			if (settingsWindow.facetHistSet.recFlightDist == 3) settingsWindow.facetHistSet.recFlightDist = toggle;
			else if (settingsWindow.facetHistSet.recFlightDist != toggle) settingsWindow.facetHistSet.recFlightDist = 2;

			settingsWindow.facetHistSet.maxFlightDist = f->sh.facetHistogramParams.distanceMax;
			tmp = fmt::format("{}", settingsWindow.facetHistSet.maxFlightDist);
			if (settingsWindow.facetHistSet.maxFlightDistInput == "") {
				settingsWindow.facetHistSet.maxFlightDistInput = tmp; // first checked facet
			}
			else if (settingsWindow.facetHistSet.maxFlightDistInput != tmp) {
				settingsWindow.facetHistSet.maxFlightDistInput = "...";
			}
			settingsWindow.facetHistSet.distBinSize = f->sh.facetHistogramParams.distanceBinsize;
			tmp = fmt::format("{}", settingsWindow.facetHistSet.distBinSize);
			if (settingsWindow.facetHistSet.distBinSizeInput == "") {
				settingsWindow.facetHistSet.distBinSizeInput = tmp; // first checked facet
			}
			else if (settingsWindow.facetHistSet.distBinSizeInput != tmp) {
				settingsWindow.facetHistSet.distBinSizeInput = "...";
			}
#ifdef MOLFLOW
			toggle = f->sh.facetHistogramParams.recordTime;
			if (settingsWindow.facetHistSet.recTime == 3) settingsWindow.facetHistSet.recTime = toggle;
			else if (settingsWindow.facetHistSet.recTime != toggle) settingsWindow.facetHistSet.recTime = 2;
			settingsWindow.facetHistSet.maxFlightTime = f->sh.facetHistogramParams.timeMax;
			tmp = fmt::format("{}", settingsWindow.facetHistSet.maxFlightTime);
			if (settingsWindow.facetHistSet.maxFlightTimeInput == "") {
				settingsWindow.facetHistSet.maxFlightTimeInput = tmp; // first checked facet
			}
			else if (settingsWindow.facetHistSet.maxFlightTimeInput != tmp) {
				settingsWindow.facetHistSet.maxFlightTimeInput = "...";
			}
			settingsWindow.facetHistSet.timeBinSize = f->sh.facetHistogramParams.timeBinsize;
			tmp = fmt::format("{}", settingsWindow.facetHistSet.timeBinSize);
			if (settingsWindow.facetHistSet.timeBinSizeInput == "") {
				settingsWindow.facetHistSet.timeBinSizeInput = tmp; // first checked facet
			}
			else if (settingsWindow.facetHistSet.timeBinSizeInput != tmp) {
				settingsWindow.facetHistSet.timeBinSizeInput = "...";
			}
#endif
		}
	}
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

void ImHistogramPlotter::DrawPlot()
{
	if(plotTab==bounces) xAxisName = "Number of bounces";
	if(plotTab==distance) xAxisName = "Distance [cm]";
#ifdef MOLFLOW
	if(plotTab==time) xAxisName = "Time [s]";
#endif
	ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, 2);
	if (ImPlot::BeginPlot("##Histogram", xAxisName.c_str(), 0, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 7 * txtH), 0, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
		if (logX) ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);
		if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
		for (auto& plot : data[plotTab]) {
			if (!plot.x || !plot.y || plot.x->size()==0 || plot.y->size()==0) continue;
			std::string name = plot.id == -1 ? "Global" : ("Facet #" + std::to_string(plot.id + 1));
			ImPlot::PlotScatter(name.c_str(), plot.x->data(), plot.y->data(), plot.x->size());
			plot.color = ImPlot::GetLastItemColor();
		}
		bool isGlobal = (globals[plotTab].x.get() != nullptr && globals[plotTab].y.get() != nullptr);
		if (isGlobal) {
			ImPlot::PlotScatter("Global", globals[plotTab].x->data(), globals[plotTab].y->data(), globals[plotTab].x->size());
			globals[plotTab].color = ImPlot::GetLastItemColor();
		}
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
	if (IsPlotted(plotTab, comboSelection)) return;
	ImPlotData newPlot;
	if (comboSelection != -1) newPlot.id = comboSelection;
	else newPlot.id = plotTab;
	
	newPlot.x = std::make_shared<std::vector<double>>();
	newPlot.y = std::make_shared<std::vector<double>>();

	if (comboSelection != -1) {
		data[plotTab].push_back(newPlot);
		RefreshPlots();
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
			plot.y = std::make_shared<std::vector<double>>(interfGeom->GetFacet(facetId)->facetHistogramCache.nbHitsHistogram); // yaxis
			if (plot.y.get() == nullptr) break;
			xMax = (double)interfGeom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceMax;
			xSpacing = interfGeom->GetFacet(facetId)->sh.facetHistogramParams.nbBounceBinsize;
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
	long preExportSel = comboSelection;
	bool preExportLimitPoints = limitPoints;
	if (!plottedOnly) {
		limitPoints = false;
		comboSelection = -1;
		AddPlot();
		for (const auto& id: comboOpts[plotTab]) {
			comboSelection = id;
			AddPlot();
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
	int n = std::max(exportGlobal ? globals[plotTab].x->size() : 0, data[plotTab].size() == 0 ? 0 : data[plotTab][0].x->size());
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
	comboSelection = preExportSel;
	limitPoints = preExportLimitPoints;
}

void ImHistogramPlotter::LoadHistogramSettings()
{
	comboSelection = -1;

	// global settings
	settingsWindow.globalHistSet.recBounce = mApp->worker.model->sp.globalHistogramParams.recordBounce;
	settingsWindow.globalHistSet.nbBouncesMax = mApp->worker.model->sp.globalHistogramParams.nbBounceMax;
	settingsWindow.globalHistSet.maxRecNbBouncesInput = fmt::format("{}", settingsWindow.facetHistSet.nbBouncesMax);
	settingsWindow.globalHistSet.bouncesBinSize = mApp->worker.model->sp.globalHistogramParams.nbBounceBinsize;
	settingsWindow.globalHistSet.bouncesBinSizeInput = fmt::format("{}", settingsWindow.facetHistSet.bouncesBinSize);

	settingsWindow.globalHistSet.recFlightDist = mApp->worker.model->sp.globalHistogramParams.recordDistance;
	settingsWindow.globalHistSet.maxFlightDist = mApp->worker.model->sp.globalHistogramParams.distanceMax;
	settingsWindow.globalHistSet.maxFlightDistInput = fmt::format("{}", settingsWindow.facetHistSet.maxFlightDist);
	settingsWindow.globalHistSet.distBinSize = mApp->worker.model->sp.globalHistogramParams.distanceBinsize;
	settingsWindow.globalHistSet.distBinSizeInput = fmt::format("{}", settingsWindow.facetHistSet.distBinSize);
#ifdef MOLFLOW
	settingsWindow.globalHistSet.recTime = mApp->worker.model->sp.globalHistogramParams.recordTime;
	settingsWindow.globalHistSet.maxFlightTime = mApp->worker.model->sp.globalHistogramParams.timeMax;
	settingsWindow.globalHistSet.maxFlightTimeInput = fmt::format("{}", settingsWindow.facetHistSet.maxFlightTime);
	settingsWindow.globalHistSet.timeBinSize = mApp->worker.model->sp.globalHistogramParams.timeBinsize;
	settingsWindow.globalHistSet.timeBinSizeInput = fmt::format("{}", settingsWindow.facetHistSet.timeBinSize);
#endif
	// combo lists
	size_t n = interfGeom->GetNbFacet();
	for (size_t i = 0; i < n; i++) {
		const auto& facet = interfGeom->GetFacet(i);
		if (facet->facetHistogramCache.nbHitsHistogram.size() != 0 && !Contains(comboOpts[bounces],i)) comboOpts[bounces].push_back(i);
		if (facet->facetHistogramCache.distanceHistogram.size() != 0 && !Contains(comboOpts[distance], i)) comboOpts[distance].push_back(i);
#ifdef MOLFLOW
		if (facet->facetHistogramCache.timeHistogram.size() != 0 && !Contains(comboOpts[time], i)) comboOpts[time].push_back(i);
#endif
	}
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
	comboOpts[bounces].clear();
	comboOpts[distance].clear();
#ifdef MOLFLOW
	comboOpts[time].clear();
#endif
	std::vector<size_t> facets = interfGeom->GetAllFacetIndices();
	for (size_t idx : facets) { // get all facets which have histograms
		InterfaceFacet* f = interfGeom->GetFacet(idx);
		if (f->sh.facetHistogramParams.recordBounce) {
			comboOpts[bounces].push_back(idx);
		}
		if (f->sh.facetHistogramParams.recordDistance) {
			comboOpts[distance].push_back(idx);
		}
#ifdef MOLFLOW
		if (f->sh.facetHistogramParams.recordTime) {
			comboOpts[time].push_back(idx);
		}
#endif
	}
	for (short tab = 0; tab < IM_HISTOGRAM_TABS; tab++) { // remove deleted facets from plot
		for (int i = data[tab].size()-1; i >= 0; --i) {
			if (!Contains(comboOpts[tab], data[tab][i].id)) {
				data[tab].erase(data[tab].begin() + i);
			}
		}
	}
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
	ImGui::SetNextWindowSize(ImVec2(width,32*txtH));
	ImGui::Begin("Histogram settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	float childHeight = (ImGui::GetContentRegionAvail().y - 1.5*txtH) * 0.5;

	if (ImGui::BeginChild("Global histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Global histogram");
		globalHistSet.amIDisabled = false;
		DrawSettingsGroup(globalHistSet, false);
	}
	ImGui::EndChild();
	if (ImGui::BeginChild("Facet histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Facet histogram");
		facetHistSet.amIDisabled = interfGeom->GetNbSelectedFacets() == 0;
		if (facetHistSet.amIDisabled) {
			ImGui::BeginDisabled();
		}
		
		// internal ImGui structure for data storage
		DrawSettingsGroup(facetHistSet, interfGeom->GetNbSelectedFacets()>1);

		if (facetHistSet.amIDisabled) ImGui::EndDisabled();
	}
	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter(" Apply ");
	if (ImGui::Button("Apply")) Apply();
	ImGui::End();
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
	if (globalHistSet.recBounce==1) {
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
	if (globalHistSet.recFlightDist==1) {
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
	if (globalHistSet.recTime==1) {
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
	if (facetHistSet.recBounce==1) {
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
	if (facetHistSet.recFlightDist==1) {
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
	if (facetHistSet.recTime==1) {
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
	
	
	if (globalHistSet.recBounce != 2) mApp->worker.model->sp.globalHistogramParams.recordBounce = globalHistSet.recBounce;
	if (globalHistSet.maxRecNbBouncesInput != "...") mApp->worker.model->sp.globalHistogramParams.nbBounceMax = globalHistSet.nbBouncesMax;
	if (globalHistSet.bouncesBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.nbBounceBinsize = globalHistSet.bouncesBinSize;
	if (globalHistSet.recFlightDist != 2) mApp->worker.model->sp.globalHistogramParams.recordDistance = globalHistSet.recFlightDist;
	if (globalHistSet.maxFlightDistInput != "...") mApp->worker.model->sp.globalHistogramParams.distanceMax = globalHistSet.maxFlightDist;
	if (globalHistSet.distBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.distanceBinsize = globalHistSet.distBinSize;

	if (!mApp->worker.model->sp.globalHistogramParams.recordBounce) 
	{
		parent->globals[bounces].x.reset();
		parent->globals[bounces].y.reset();
	}
	if (!mApp->worker.model->sp.globalHistogramParams.recordDistance) 
	{
		parent->globals[distance].x.reset();
		parent->globals[distance].y.reset();
	}
#ifdef MOLFLOW
	if (globalHistSet.recTime != 2) mApp->worker.model->sp.globalHistogramParams.recordTime = globalHistSet.recTime;
	if (globalHistSet.maxFlightTimeInput != "...") mApp->worker.model->sp.globalHistogramParams.timeMax = globalHistSet.maxFlightTime;
	if (globalHistSet.timeBinSizeInput != "...") mApp->worker.model->sp.globalHistogramParams.timeBinsize = globalHistSet.timeBinSize;

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
		if (facetHistSet.recBounce != 2) f->sh.facetHistogramParams.recordBounce = facetHistSet.recBounce;
		if (facetHistSet.maxRecNbBouncesInput != "...") f->sh.facetHistogramParams.nbBounceMax = facetHistSet.nbBouncesMax;
		if (facetHistSet.bouncesBinSizeInput != "...") f->sh.facetHistogramParams.nbBounceBinsize = facetHistSet.bouncesBinSize;
		if (facetHistSet.recFlightDist != 2) f->sh.facetHistogramParams.recordDistance = facetHistSet.recFlightDist;
		if (facetHistSet.maxFlightDistInput != "...") f->sh.facetHistogramParams.distanceMax = facetHistSet.maxFlightDist;
		if (facetHistSet.distBinSizeInput != "...") f->sh.facetHistogramParams.distanceBinsize = facetHistSet.distBinSize;
#ifdef MOLFLOW
		if (facetHistSet.recTime != 2) f->sh.facetHistogramParams.recordTime = facetHistSet.recTime;
		if (facetHistSet.maxFlightTimeInput != "...") f->sh.facetHistogramParams.timeMax = facetHistSet.maxFlightTime;
		if (facetHistSet.timeBinSizeInput != "...") f->sh.facetHistogramParams.timeBinsize = facetHistSet.timeBinSize;
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
	return true;
}

void ImHistogramPlotter::ImHistogramSettings::DrawSettingsGroup(histSet& set, bool tristate)
{
	ImGui::TriState("Record bounces until absorbtion", &set.recBounce, tristate);
	if (!set.amIDisabled && set.recBounce!=1) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded no. of bounces:", &set.maxRecNbBouncesInput,0,txtW*6);
	ImGui::InputTextRightSide("Bounces bin size:", &set.bouncesBinSizeInput,0,txtW*6);
	if (!set.amIDisabled && set.recBounce != 1) ImGui::EndDisabled();

	ImGui::TriState("Record flight distance until absorption", &set.recFlightDist, tristate);
	if (!set.amIDisabled && set.recFlightDist != 1) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded flight distance (cm):", &set.maxFlightDistInput, 0, txtW * 6);
	ImGui::InputTextRightSide("Distance bin size (cm):", &set.distBinSizeInput, 0, txtW * 6);
	if (!set.amIDisabled && set.recFlightDist != 1) ImGui::EndDisabled();
#ifdef MOLFLOW
	ImGui::TriState("Record flight time until absorption", &set.recTime, tristate);
	if (!set.amIDisabled && set.recTime != 1) ImGui::BeginDisabled();
	ImGui::InputTextRightSide("Max recorded flight time (s):", &set.maxFlightTimeInput, 0, txtW * 6);
	ImGui::InputTextRightSide("Time bin size (s):", &set.timeBinSizeInput, 0, txtW * 6);
	if (!set.amIDisabled && set.recTime != 1) ImGui::EndDisabled();
#endif
	// to the best of my understanding, this does not do anything in the Legacy code
	// if that is the case it can be removed
	ImGui::Text("Memory estimate of histogram: " + (set.showMemEst ? std::to_string(set.memEst) : ""));
}
