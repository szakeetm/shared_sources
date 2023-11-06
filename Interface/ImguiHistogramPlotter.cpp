#include "ImguiHistogramPlotter.h"
#include "imgui.h"
#include "implot/implot.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"

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
	}
	ImGui::EndChild();
	if (ImGui::BeginChild("Facet histogram", ImVec2(0, childHeight), true)) {
		ImGui::TextDisabled("Facet histogram");
		facetHistSet.amIDisabled = interfGeom->GetNbSelectedFacets() == 0;
		if (facetHistSet.amIDisabled) {
			ImGui::BeginDisabled();
		}
		
		// internal ImGui structure for data storage
		Settings(facetHistSet);

		if (facetHistSet.amIDisabled) ImGui::EndDisabled();
	}
	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter(" Apply ");
	if (ImGui::Button("Apply")) Apply();
	ImGui::End();
}

bool ImHistogramPlotter::ImHistagramSettings::Apply()
{
	// global

	if (globalHistSet.globalRecBounce) {
		if (globalHistSet.maxRecNbBouncesInput == "...") {} // empty if just to pass this check when "..." is input, todo this is an ugly solution, restructure the if
		else if (!Util::getNumber(&globalHistSet.nbBouncesMax,globalHistSet.maxRecNbBouncesInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global bounce limit");
			return false;
		}
		else if (globalHistSet.nbBouncesMax <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global bounce limit must be a non-negative integer");
			return false;
		}
		if (globalHistSet.bouncesBinSizeInput == "...") {}
		else if (!Util::getNumber(&globalHistSet.bouncesBinSize, globalHistSet.bouncesBinSizeInput)) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Invalid input in global bounce bin size");
			return false;
		}
		else if (globalHistSet.bouncesBinSize <= 0) {
			ImIOWrappers::InfoPopup("Histogram parameter error", "Global bounce bin size must be a positive integer");
			return false;
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

void ImHistogramPlotter::ImHistagramSettings::Settings(histSet& set)
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

	ImGui::Text("Memory estimate of histogram: " + (set.showMemEst ? std::to_string(set.memEst) : ""));
}
