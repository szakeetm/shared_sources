#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "ImguiConvergencePlotter.h"
#include <imgui.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot/implot.h"
#include "implot/implot_internal.h"
#include "Buffer_shared.h"
#include <functional>
#include "Helper/MathTools.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "ImguiExtensions.h"

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
	for (int i = onlyVisible && data[0].x->size()>maxDatapoints ? data[0].x->size() - maxDatapoints : 0; i < data[0].x->size(); i++) {
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
			out.append(fmt::format("{}", formula.y->at(i)) + "\t");
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
			if (ImGui::InputFloat("##lineWidth", &lineWidth, 0.1, 1, "%.2f")) {
				if (lineWidth < 0.5) lineWidth = 0.5;
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
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Custom Plot")) {
			ImGui::SetNextItemWidth(txtW * 15);
			ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
			if (ImGui::Button("-> Plot expression")) {
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
			else if (drawManual && ImGui::Button("Apply")) {
				manualxValues.clear();
				manualyValues.clear();
				ImUtils::ComputeManualExpression(drawManual, formula, manualxValues, manualyValues, endX, startX, step);
			}
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
	if (selectedFormula == idx) selectedFormula = -1;
	for (int i = 0; i < data.size(); i++) if (data[i].id == idx)
	{
		data.erase(data.begin() + i);
		break;
	}
}

void ImConvergencePlotter::Reload()
{
	data.clear();
	selectedFormula = -1;
}

void ImConvergencePlotter::LoadSettingsFromFile(bool log, std::vector<int> plotted)
{
	data.clear();
	logY = log;
	for (int id : plotted) {
		id += 494667622;
		if (IsPlotted(id)) continue;
		if (id >= 0 && id < mApp->appFormulas->formulas.size()) {
			if (mApp->appFormulas->formulas[id].hasEvalError) continue;
			else {
				this->data.push_back(ImUtils::MakePlotData(id));
			}
		}
	}
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
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 90, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Convergence Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar);
	
	MenuBar();
	int nFormulas = mApp->appFormulas->formulas.size();
	formulaDrawToggle.resize(nFormulas, false);

	ImGui::BeginChild("Sidebar", ImVec2(txtW*20, ImGui::GetContentRegionAvail().y), true);
	static short aggregateState = 0;
	static bool mixedState = false;
	//if(ImGui::TriState("All", &aggregateState, mixedState)) {}
	for (int i = 0; i < nFormulas; i++) {
		std::string fName = ("[" + mApp->appFormulas->formulas[i].GetName() + "]" + mApp->appFormulas->formulas[i].GetExpression());
		if (ImGui::Checkbox(fName.c_str(), (bool*)&(formulaDrawToggle[i]))) {
			if (formulaDrawToggle[i] == 0 && IsPlotted(i)) RemovePlot(i);
			else if (formulaDrawToggle[i] == 1 && !IsPlotted(i)) data.push_back(ImUtils::MakePlotData(i));
			if (formulaDrawToggle[i] != aggregateState) {
				aggregateState = 2;
				mixedState = true;
			}
		}
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();
	DrawConvergenceGraph();
	ImGui::SetNextItemWidth(txtW*30);
	

	if (nFormulas == 0) ImGui::BeginDisabled();
	if (ImGui::BeginCombo("##Formula Picker",
			nFormulas==0 ? "No formula found" :
		selectedFormula==-1 || selectedFormula>=nFormulas ? "Choose Formula" :
		("[" + mApp->appFormulas->formulas[selectedFormula].GetName() + "]" + mApp->appFormulas->formulas[selectedFormula].GetExpression()).c_str())) {
		for (int i = 0; i < nFormulas; i++) {
			if (ImGui::Selectable(("[" + mApp->appFormulas->formulas[i].GetName() + "]" + mApp->appFormulas->formulas[i].GetExpression()).c_str(),
				i == selectedFormula)) {
				selectedFormula = i;
			}
		}
		ImGui::EndCombo();
	}
	if (nFormulas == 0) ImGui::EndDisabled();
	
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvail().x - txtW * (33.5+3);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();

	if (ImGui::Button("Add curve")) {
		if (IsPlotted(selectedFormula)) {
			ImIOWrappers::InfoPopup("Info", "Profile already plotted");
		}
		else if (selectedFormula != -1) {
			if (mApp->appFormulas->formulas[selectedFormula].hasEvalError) {
				ImIOWrappers::InfoPopup("Error", fmt::format("Formula can't be evaluated:\n{}", mApp->appFormulas->formulas[selectedFormula].GetEvalErrorMsg()));
			}
			else data.push_back(ImUtils::MakePlotData(selectedFormula));
		}
	} ImGui::SameLine();
	if (ImGui::Button("Remove curve")) {
		if (IsPlotted(selectedFormula)) {
			for (int i=0;i<data.size();i++) if (data[i].id==selectedFormula)
			{
				data.erase(data.begin() + i);
				break;
			}
		}
		else ImIOWrappers::InfoPopup("Error", "Profile not plotted");
	} ImGui::SameLine();
	if (ImGui::Button("Remove all")) 
	{
		data.clear();
		drawManual = false;
		manualxValues.clear();
		manualyValues.clear();
	}
	ImGui::SameLine();
	ImGui::HelpMarker("Right-click plot to adjust fiting, Scaling etc.\nScroll to zoom\nHold and drag to move (auto-fit must be disabled first)\nHold right and drag for box select (auto-fit must be disabled first)");
	ImGui::EndGroup();
	ImGui::End();
}

void ImConvergencePlotter::DrawConvergenceGraph()
{
	lockYtoZero = data.size() == 0 && !drawManual;
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight,lineWidth);
	ImPlot::SetNextAxesLimits(0, maxDatapoints, 0, maxDatapoints, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##Convergence","Number of desorptions",0,ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetWindowSize().y-4.5*txtH),0, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
		if (logY) ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);
		for (int i = 0; i < data.size(); i++) {
			if (mApp->appFormulas->convergenceData.size() <= i) break;
			const std::vector<FormulaHistoryDatapoint>& values = mApp->appFormulas->convergenceData[data[i].id];
			actualNbValues = values.size();
			int count = values.size()>maxDatapoints ? maxDatapoints : values.size();
			data[i].x->clear();
			data[i].y->clear();
			for (int j = 0; j < values.size(); j++) {
				data[i].x->push_back(values[j].nbDes);
				data[i].y->push_back(values[j].value);
			}
			if (data[i].x->size() == 0) continue;
			std::string name = mApp->appFormulas->formulas[data[i].id].GetName();
			if(name=="") name = mApp->appFormulas->formulas[data[i].id].GetExpression();
			if (showDatapoints) ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
			ImPlot::PlotLine(name.c_str(), data[i].x->data(), data[i].y->data(),count);
		}

		//draw manual
		if (showDatapoints && drawManual) ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle);
		if (drawManual)	ImPlot::PlotLine(formula.GetName().c_str(), manualxValues.data(), manualyValues.data(), manualxValues.size());
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