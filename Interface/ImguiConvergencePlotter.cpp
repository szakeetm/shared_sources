#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "ImguiConvergencePlotter.h"
#include "imgui.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot.h"
#include "implot_internal.h"
#include "Buffer_shared.h"
#include <functional>
#include "Helper/MathTools.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"

void ImConvergencePlotter::Init(Interface* mApp_) {
	mApp = mApp_;
}

bool ImConvergencePlotter::Export(bool toFile, bool onlyVisible)
{
	if (data.size() == 0) {
		ImIOWrappers::InfoPopup("Error", "Nothing to export");
		return false;
	}
	std::string out;
	// first row (headers)
	out.append("X axis\t");
	if (drawManual) out.append("manual\t");
	for (const auto& formula : data) {
		out.append(fmt::format("[{}]{}\t",mApp->appFormulas->formulas[formula.id].GetName(), mApp->appFormulas->formulas[formula.id].GetExpression()));
	}
	out[out.size() - 1] = '\n';
	// rows
	for (size_t i = (onlyVisible && data[0].x->size() > maxDatapoints) ? data[0].x->size() - maxDatapoints : 0; i < data[0].x->size(); i++) {
		out.append(fmt::format("{}", data[0].x->at(i)) + "\t");
		if (drawManual) {
			if (formula.GetNbVariable() != 0) {
				std::list<Variable>::iterator xvar = formula.GetVariableAt(0);
				xvar->value = data[0].x->at(i);
			}
			double yvar = formula.Evaluate();
			out.append(fmt::format("{}\t", yvar));
		}
		for (const auto& formula : data) {
			if (formula.y->size() > i) out.append(fmt::format("{}", formula.y->at(i)) + "\t");
			else (out.append("\t"));
		}
		out[out.size() - 1] = '\n';
	}
	if (!toFile) SDL_SetClipboardText(out.c_str());
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

void ImConvergencePlotter::MenuBar()
{
	if (ImGui::BeginMenuBar()) {
		if (ImGui::BeginMenu("Export")) {
			if (ImGui::MenuItem("All to clipboard")) Export();
			if (ImGui::MenuItem("All to file")) Export(true);
			ImGui::Separator();
			if (ImGui::MenuItem("Plotted to clipboard")) Export(false, true);
			if (ImGui::MenuItem("Plotted to file")) Export(true, true);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			//ImGui::Checkbox("Colorblind mode", &colorBlind);
			ImGui::Checkbox("Log Y", &logY);
			ImGui::Checkbox("Datapoints", &showDatapoints);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Linewidth:");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(txtW * 12);
			if (ImGui::InputFloat("##lineWidth", &lineWidth, 0.1f, 1.f, "%.2f")) {
				if (lineWidth < 0.5f) lineWidth = 0.5f;
			}
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Limit"); ImGui::SameLine();
			ImGui::SetNextItemWidth(15 * txtW);
			if (ImGui::InputInt("##plotMax", &maxDatapoints, 100, 1000)) {
				if (maxDatapoints < 0) maxDatapoints = 0;
			}
			ImGui::Checkbox("Display hovered value", &showValueOnHover);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Data")) {
			static int N=2;
			ImGui::AlignTextToFramePadding();
			ImGui::Text("N"); ImGui::SameLine();
			ImGui::SetNextItemWidth(15*txtW);
			if (ImGui::InputInt("##N", &N, 1, 100)) {
				if (N < 1) N = 1;
			}
			if (ImGui::MenuItem(fmt::format("Remove every Nth element"))) {
				for (int formulaId = 0; formulaId < mApp->appFormulas->formulas.size(); ++formulaId) {
					mApp->appFormulas->removeEveryNth(N, formulaId, 0);
				}
			}
			if (ImGui::MenuItem(fmt::format("Remove the first N elements"))) {
				for (int formulaId = 0; formulaId < mApp->appFormulas->formulas.size(); ++formulaId) {
					mApp->appFormulas->removeFirstN(N, formulaId);
				}
			}
			if (ImGui::MenuItem("Show formula editor")) {
				mApp->imWnd->formulaEdit.Show();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Custom Plot")) {
			ImGui::SetNextItemWidth(txtW * 15);
			ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
			if (ImGui::Button("-> Plot expression") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
				drawManual = ImUtils::ParseExpression(expression, formula);
				manualxValues.clear();
				manualyValues.clear();
				if (drawManual) ImUtils::ComputeManualExpression(drawManual, formula, manualxValues, manualyValues, endX, startX, step);
			}
			ImGui::InputDoubleRightSide("StartX", &startX, "%.2f");
			ImGui::InputDoubleRightSide("EndX", &endX, "%.2f");
			if (ImGui::InputDoubleRightSide("Step", &step, "%.4f")) {
				if (step < 1e-4) step = 1e-4;
			}
			if (endX - startX < 0) ImGui::TextDisabled("Start X cannot be higer than End X");
			if (abs(endX - startX) / step > 1e5) ImGui::TextColored(ImVec4(1, 0, 0, 1), fmt::format("Warning! Number of datapoints\nwill exceed {}!", 1e5).c_str());
			ImGui::EndMenu();
		}

		if (data.size()!=0 && maxDatapoints< actualNbValues) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0, 0, 1), fmt::format("   Warning! Showing the first {} of {} values", maxDatapoints, actualNbValues).c_str());
		}

		ImGui::EndMenuBar();
	}
}

void ImConvergencePlotter::RemovePlot(int idx) {
	for (int i = 0; i < data.size(); i++) if (data[i].id == idx)
	{
		data.erase(data.begin() + i);
		break;
	}
}

void ImConvergencePlotter::AddPlot(int idx)
{
	data.push_back(ImUtils::MakePlotData(idx));
	GetData();
}

void ImConvergencePlotter::OnShow()
{
	Refresh();
}

void ImConvergencePlotter::Refresh()
{
	nFormulas = mApp->appFormulas->formulas.size();
	formulaDrawToggle.resize(nFormulas, false);
	GetData();
	UpdateSidebarMasterToggle();
}

void ImConvergencePlotter::GetData() {
	for (int i = 0; i < data.size(); i++) {
		if (mApp->appFormulas->convergenceData.size() <= data[i].id) continue;
		const std::vector<FormulaHistoryDatapoint>& values = mApp->appFormulas->convergenceData[data[i].id];
		actualNbValues = values.size();
		size_t count = values.size() > maxDatapoints ? maxDatapoints : values.size();
		data[i].x->clear();
		data[i].y->clear();
		for (int j = 0; j < values.size(); j++) {
			data[i].x->push_back(static_cast<double>(values[j].nbDes));
			data[i].y->push_back(values[j].value);
		}
	}
}

void ImConvergencePlotter::LoadSettingsFromFile(bool log, std::vector<int> plotted)
{
	data.clear();
	logY = log;
	for (int id : plotted) {
		id += 494667622; // offset from unsigned to signed
		if (IsPlotted(id)) continue;
		if (id >= 0 && id < mApp->appFormulas->formulas.size()) {
			if (mApp->appFormulas->formulas[id].hasEvalError) continue;
			else {
				this->data.push_back(ImUtils::MakePlotData(id));
			}
		}
	}
	UpdateSidebarMasterToggle();
}

//pass the first ID value which should be changed
void ImConvergencePlotter::DecrementFormulaIndicies(int startId)
{
	for (auto& formula : data) {
		if (formula.id >= startId) formula.id--;
	}
}

void ImConvergencePlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(30*txtW, 40*txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 82, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Convergence Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
	
	MenuBar();

	ImGui::BeginChild("Sidebar", ImVec2(txtW*20, ImGui::GetContentRegionAvail().y), true);
	if(ImGui::TriState("All", &aggregateState, mixedState)) {
		ApplyAggregateState();
	} ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvail().x - txtW * 3;
	ImGui::Dummy(ImVec2(dummyWidth,0));
	ImGui::SameLine();
	ImGui::HelpMarker("Right-click plot or axis to adjust fiting\nScroll to zoom (with auto-fit off)\nHold and drag to move (auto-fit must be off)\nHold right and drag for box select (auto-fit must be off)\nToggle logarithmic Y axis in View menu");
	ImGui::Separator();
	for (int i = 0; i < nFormulas; i++) {
		std::string fName = ("[" + mApp->appFormulas->formulas[i].GetName() + "]" + mApp->appFormulas->formulas[i].GetExpression());
		if (ImGui::Checkbox(fName.c_str(), (bool*)&(formulaDrawToggle[i]))) {
			if (formulaDrawToggle[i] == 0 && IsPlotted(i)) RemovePlot(i);
			else if (formulaDrawToggle[i] == 1 && !IsPlotted(i)) AddPlot(i);
			UpdateSidebarMasterToggle();
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	DrawConvergenceGraph();
	ImGui::End();
}

void ImConvergencePlotter::DrawConvergenceGraph()
{
	lockYtoZero = data.size() == 0 && !drawManual;
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight,lineWidth);
	ImPlot::SetNextAxesLimits(0, maxDatapoints, 0, maxDatapoints, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##Convergence", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y - 4.5f * txtH))) {
		ImPlot::SetupAxis(ImAxis_X1, "Number of desorptions", ImPlotAxisFlags_AutoFit);
		ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_AutoFit);
		if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
		for (int i = 0; i < data.size(); i++) {
			if (data[i].x == nullptr || data[i].y == nullptr || data[i].x->size() != data[i].y->size()) continue;
			if (mApp->appFormulas->formulas.size() <= data[i].id) continue;
			std::string name = mApp->appFormulas->formulas[data[i].id].GetName();
			if(name=="") name = mApp->appFormulas->formulas[data[i].id].GetExpression();
			if (showDatapoints) ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::PlotLine(name.c_str(), data[i].x->data(), data[i].y->data(), static_cast<int>(data[i].x->size()));
		}

		//draw manual
		if (showDatapoints && drawManual) ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		if (drawManual)	ImPlot::PlotLine(formula.GetName().c_str(), manualxValues.data(), manualyValues.data(), static_cast<int>(manualxValues.size()));
		// value tooltip
		if(showValueOnHover) ImUtils::DrawValueOnHover(data, drawManual, &manualxValues, &manualyValues);
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	if (colorBlind) ImPlot::PopColormap();
	/*
	if (lockYtoZero) {
		ImPlotPlot& thisPlot = *ImPlot::GetPlot("##Convergence");
		thisPlot.YAxis->SetMin(0, true);
		thisPlot.XAxis.SetMin(0, true);
		lockYtoZero = false;
	}
	*/
}

bool ImConvergencePlotter::IsPlotted(size_t idx)
{
	for (const auto& formula : data) {
		if (idx == formula.id) return true;
	}
	return false;
}

void ImConvergencePlotter::UpdateSidebarMasterToggle()
{
	mixedState = false;
	if (nFormulas == 0) {
		aggregateState = 0;
		return;
	}
	formulaDrawToggle.resize(nFormulas, false);
	aggregateState = formulaDrawToggle[0];
	for (int i = 1; i < nFormulas; i++) {
		if (aggregateState != formulaDrawToggle[i]) {
			aggregateState = 2;
			mixedState = true;
			return;
		}
	}
}

void ImConvergencePlotter::ApplyAggregateState()
{
	nFormulas = mApp->appFormulas->formulas.size();
	formulaDrawToggle.resize(nFormulas, false);
	mixedState = false;
	for (int i = 0; i < nFormulas; i++) {
		formulaDrawToggle[i] = aggregateState;
		if (formulaDrawToggle[i] == 0 && IsPlotted(i)) RemovePlot(i);
		else if (formulaDrawToggle[i] == 1 && !IsPlotted(i)) data.push_back(ImUtils::MakePlotData(i));
	}
}
