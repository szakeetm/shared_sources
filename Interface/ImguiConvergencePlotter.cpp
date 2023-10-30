#include "ImguiConvergencePlotter.h"
#include <imgui.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot/implot.h"
#include "Buffer_shared.h"
#include <functional>
#include "Helper/MathTools.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"

void ImConvergencePlotter::Init(Interface* mApp_) {
	mApp = mApp_;
	ImPlot::GetStyle().AntiAliasedLines = true;
}

bool ImConvergencePlotter::PlotNewExpression() {
	// Could not find documentation or help on how this is supposed to behave
	// Behaviour copied from old gui without full understanding of it
	// Verification needed
	// It seems legacy was not working
	formula = GLFormula();
	formula.SetExpression(expression);
	if (!formula.Parse()) {
		ImIOWrappers::InfoPopup("Error", formula.GetParseErrorMsg());
		return false;
	}
	int nbVar = formula.GetNbVariable();
	if (nbVar == 0) {
		ImIOWrappers::InfoPopup("Error", "Variable x not found");
		return false;
	}
	if (nbVar > 1) {
		ImIOWrappers::InfoPopup("Error", "Too many varuables or an unknown constant"); // Legacy had grammar issiues
		return false;
	}
	auto xVariable = formula.GetVariableAt(0);
	if (!iequals(xVariable->varName, "x")) {
		ImIOWrappers::InfoPopup("Error", "Variable x not found");
		return false;
	}
	formula.SetName(expression);
	return true;
}

void ImConvergencePlotter::computeManual()
{
	std::vector<double>().swap(manualxValues);
	std::vector<double>().swap(manualyValues);
	auto xVariable = formula.GetVariableAt(0);
	for (double i = startX; i < endX; i+=step) {
		xVariable->value = static_cast<double>(i);
		double y;
		try {
			y = formula.Evaluate();
		}
		catch (...) {
			if (formula.hasEvalError) std::cout << formula.evalErrorMsg << std::endl;
			formula.hasEvalError = false;
			continue; //Eval. error, but maybe for other x it is possible to evaluate (ex. div by zero)
		}
		manualyValues.push_back(y);
		manualxValues.push_back(i);
	}
	if (manualxValues.size() == 0) {
		ImIOWrappers::InfoPopup("Error", "Error evaluating formula");
		drawManual = false;
	}
}

void ImConvergencePlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(3*txtW, 4*txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 77, txtH * 15), ImVec2(1000 * txtW, 100 * txtH));
	ImGui::Begin("Convergence Plotter", &drawn, ImGuiWindowFlags_NoSavedSettings);
	
	DrawConvergenceGraph();

	ImGui::SetNextItemWidth(txtW*30);
	static int selectedFormula = -1;
	
	int nFormulas = mApp->appFormulas->formulas.size();

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
	//ImGui::SameLine();
	//ImGui::Checkbox("Log Y", &logY); // available by right click in ImPlot
	
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (33.5);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();

	if (ImGui::Button("Add curve")) {
		// todo add check if already added
		if (Contains(drawnFormulas, selectedFormula)) {
			ImIOWrappers::InfoPopup("Info", "Profile already plotted");
		} else	drawnFormulas.push_back(selectedFormula);
	} ImGui::SameLine();
	if (ImGui::Button("Remove curve")) {
		if (Contains(drawnFormulas, selectedFormula)) {
			drawnFormulas.erase(drawnFormulas.begin()+FirstIndex(drawnFormulas, selectedFormula));
		}
		else ImIOWrappers::InfoPopup("Error", "Profile not plotted");
	} ImGui::SameLine();
	if (ImGui::Button("Remove all")) 
	{
		drawnFormulas.clear();
		drawManual = false;
	}

	ImGui::Checkbox("Colorblind mode", &colorBlind);
	ImGui::SameLine();
	ImGui::Text("Change linewidth:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	if (ImGui::InputFloat("##lineWidth", &lineWidth, 0.1, 1,"%.2f")) {
		if (lineWidth < 0.5) lineWidth = 0.5;
	}
	
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (31);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();

	if (ImGui::Button("Remove every 4th")) {
		for (int formulaId = 0; formulaId < mApp->appFormulas->formulas.size(); ++formulaId) {
			mApp->appFormulas->removeEveryNth(4, formulaId, 0);
		}
	} ImGui::SameLine();
	if (ImGui::Button("Remove first 100")) {
		for (int formulaId = 0; formulaId < mApp->appFormulas->formulas.size(); ++formulaId) {
			mApp->appFormulas->removeFirstN(100, formulaId);
		}
	}
	
	ImGui::SetNextItemWidth(txtW * 40);
	ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
	if (ImGui::Button("-> Plot expression")) {
		drawManual = PlotNewExpression();
		computeManual();
	}
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Start X:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	ImGui::InputInt("##StartX", &startX); ImGui::SameLine();
	ImGui::Text("End X:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	ImGui::InputInt("##EndX", &endX); ImGui::SameLine();
	ImGui::Text("Step size:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 12);
	ImGui::InputDouble("##Step", &step,1,4,"%.2f");
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (8.25);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	if (ImGui::Button("Dismiss")) { Hide(); }

	ImGui::End();
}

void ImConvergencePlotter::DrawConvergenceGraph()
{
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight,lineWidth);
	ImPlot::SetNextPlotLimits(0, 1000, 0, 1000, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##Convergence","Number of desorptions",0,ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y-7.5*txtH))) {
		for (int i = 0; i < drawnFormulas.size(); i++) {
			const std::vector<FormulaHistoryDatapoint>& data = mApp->appFormulas->convergenceData[drawnFormulas[i]];
			std::vector<double> xValues, yValues;
			int count = data.size()>1000 ? 1000 : data.size();
			for (int j = std::max(0, (int)data.size() - 1000); j < data.size(); j++) {
				xValues.push_back(data[j].nbDes);
				yValues.push_back(data[j].value);
			}
			std::string name = mApp->appFormulas->formulas[drawnFormulas[i]].GetName();
			if(name=="") name = mApp->appFormulas->formulas[drawnFormulas[i]].GetExpression();
			ImPlot::PlotLine(name.c_str(), xValues.data(), yValues.data(),count);
		}

		//draw manual
		if (drawManual) {
			ImPlot::PlotLine(formula.GetName().c_str(), manualxValues.data(), manualyValues.data(), manualxValues.size());
		}

		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	if (colorBlind) ImPlot::PopColormap();
}
