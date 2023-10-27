#include "ImguiConvergencePlotter.h"
#include <imgui.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot/implot.h"
#include "Buffer_shared.h"
#include <functional>
#include "Helper/MathTools.h"
#include "ImguiPopup.h"

void ImConvergencePlotter::Init(Interface* mApp_) {
	mApp = mApp_;
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
	if (ImGui::Button("Remove all")) drawnFormulas.clear();

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
	if (ImGui::Button("-> Plot expression")) {} ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (8.25);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	if (ImGui::Button("Dismiss")) {}

	ImGui::End();
}

void ImConvergencePlotter::DrawConvergenceGraph()
{
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight,lineWidth);
	if (ImPlot::BeginPlot("##Convergence","Number of desorptions",0,ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y-6*txtH))) {
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
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	if (colorBlind) ImPlot::PopColormap();
}
