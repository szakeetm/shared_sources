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
	ImPlot::GetStyle().AntiAliasedLines = true;
}

void ImConvergencePlotter::Draw()
{
	if (!drawn) return;
	float dummyWidth;
	ImGui::SetNextWindowPos(ImVec2(3*txtW, 4*txtW), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 77, txtH * 20), ImVec2(1000 * txtW, 100 * txtH));
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
		if (IsPlotted(selectedFormula)) {
			ImIOWrappers::InfoPopup("Info", "Profile already plotted");
		}
		else if (selectedFormula != -1)	drawnFormulas.push_back(ImUtils::MakePlotData(selectedFormula));
	} ImGui::SameLine();
	if (ImGui::Button("Remove curve")) {
		if (IsPlotted(selectedFormula)) {
			for (int i=0;i<drawnFormulas.size();i++) if (drawnFormulas[i].id==selectedFormula)
			{
				drawnFormulas.erase(drawnFormulas.begin() + i);
				break;
			}
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
	
	ImGui::SetNextItemWidth(txtW * 15);
	ImGui::InputText("##expressionInput", &expression); ImGui::SameLine();
	if (ImGui::Button("-> Plot expression")) {
		drawManual = ImUtils::ParseExpression(expression, formula);
		ImUtils::ComputeManualExpression(drawManual, formula, manualxValues, manualyValues, endX - startX);
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
	dummyWidth = ImGui::GetContentRegionAvailWidth() - txtW * (11.25);
	ImGui::Dummy(ImVec2(dummyWidth, txtH)); ImGui::SameLine();
	ImGui::SameLine();
	ImGui::HelpMarker("Right-click plot to adjust fiting, scailing etc.\nScroll to zoom\nHold and drag to move");
	ImGui::SameLine();
	if (ImGui::Button("Dismiss")) { Hide(); }

	ImGui::End();
}

void ImConvergencePlotter::DrawConvergenceGraph()
{
	if (colorBlind) ImPlot::PushColormap(ImPlotColormap_BrBG); // colormap without green for red-green colorblindness
	ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight,lineWidth);
	ImPlot::SetNextPlotLimits(0, 1000, 0, 1000, ImGuiCond_FirstUseEver);
	if (ImPlot::BeginPlot("##Convergence","Number of desorptions",0,ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetWindowSize().y-7.5*txtH),0, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit)) {
		for (int i = 0; i < drawnFormulas.size(); i++) {
			const std::vector<FormulaHistoryDatapoint>& data = mApp->appFormulas->convergenceData[drawnFormulas[i].id];
			int count = data.size()>1000 ? 1000 : data.size();
			drawnFormulas[i].x->clear();
			drawnFormulas[i].y->clear();
			for (int j = std::max(0, (int)data.size() - 1000); j < data.size(); j++) {
				drawnFormulas[i].x->push_back(data[j].nbDes);
				drawnFormulas[i].y->push_back(data[j].value);
			}
			std::string name = mApp->appFormulas->formulas[drawnFormulas[i].id].GetName();
			if(name=="") name = mApp->appFormulas->formulas[drawnFormulas[i].id].GetExpression();
			ImPlot::PlotLine(name.c_str(), drawnFormulas[i].x->data(), drawnFormulas[i].y->data(),count);
		}

		//draw manual
		if (drawManual) {
			ImPlot::PlotLine(formula.GetName().c_str(), manualxValues.data(), manualyValues.data(), manualxValues.size());
		}
		// value tooltip
		DrawValueOnHover();
		ImPlot::EndPlot();
	}
	ImPlot::PopStyleVar();
	if (colorBlind) ImPlot::PopColormap();
}

bool ImConvergencePlotter::IsPlotted(size_t idx)
{
	for (const auto& formula : drawnFormulas) {
		if (idx == formula.id) return true;
	}
	return false;
}

void ImConvergencePlotter::DrawValueOnHover() {
	if (ImPlot::IsPlotHovered()) {
		ImPlotPoint mouse = ImPlot::GetPlotMousePos();
		if (drawnFormulas.size() != 0 || drawManual) {
			int entryIdx = -1;
			double minYDiff = ImPlot::PlotToPixels(ImPlotPoint(0, ImPlot::GetPlotLimits().Y.Size())).y;
			int plotIdx = -2;
			for (int i = 0; i < drawnFormulas.size(); i++) { // find which plot contains the value closest to cursor
				int entryIdxTmp = ImUtils::EntryIndexFromXAxisValue(mouse.x, drawnFormulas[i]);
				double dist = abs(ImPlot::PlotToPixels(ImPlotPoint(0, mouse.y)).y - ImPlot::PlotToPixels(ImPlotPoint(0, drawnFormulas[i].y->at(entryIdxTmp))).y);
				if (entryIdxTmp != -1 && dist < minYDiff) {
					minYDiff = dist;
					plotIdx = i;
					entryIdx = entryIdxTmp;
				}
			}
			if (drawManual) {
				int entryIdxTmp = ImUtils::EntryIndexFromXAxisValue(mouse.x, manualxValues);
				double dist = abs(ImPlot::PlotToPixels(ImPlotPoint(0, mouse.y)).y - ImPlot::PlotToPixels(ImPlotPoint(0, manualyValues[entryIdxTmp])).y);
				if (entryIdxTmp != -1 && dist < minYDiff) {
					minYDiff = dist;
					plotIdx = -1;
					entryIdx = entryIdxTmp;
				}
			}
			if (plotIdx == -1) {
				entryIdx = ImUtils::EntryIndexFromXAxisValue(mouse.x, manualxValues);
			}
			else if (plotIdx >= 0 && plotIdx < drawnFormulas.size()) {
				entryIdx = ImUtils::EntryIndexFromXAxisValue(mouse.x, drawnFormulas[plotIdx]);
			}
			if (plotIdx < drawnFormulas.size() && plotIdx >= -1 && entryIdx >= 0 && entryIdx < (plotIdx < 0 ? manualxValues.size() : drawnFormulas[plotIdx].x->size())) {
				double X = plotIdx == -1 ? manualxValues[entryIdx] : drawnFormulas[plotIdx].x->at(entryIdx);
				double Y = plotIdx == -1 ? manualyValues[entryIdx] : drawnFormulas[plotIdx].y->at(entryIdx);
				ImPlot::PushStyleColor(0, ImVec4(0, 0, 0, 1));
				ImPlot::PlotScatter("", &X, &Y, 1);
				ImPlot::PopStyleColor();

				ImVec2 tooltipPos = ImPlot::PlotToPixels(ImPlotPoint(X, Y));
				if (ImPlot::GetPlotPos().y + ImPlot::GetPlotSize().y - tooltipPos.y < 2 * txtH) tooltipPos.y -= 2 * txtH;
				if (ImPlot::GetPlotPos().x + ImPlot::GetPlotSize().x - tooltipPos.x < 20 * txtW) tooltipPos.x -= 17 * txtW;

				ImGui::SetNextWindowPos(tooltipPos);
				ImGui::BeginTooltipEx(0, 0);
				ImGui::Text("X=" + std::to_string(X) + "\nY=" + std::to_string(Y));
				ImGui::EndTooltip();
			}
		}
	}
}