#include "ImguiWindowBase.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "implot/implot.h"
#include "ImguiPopup.h"
#include "Helper/StringHelper.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif

void ImWindow::Init(Interface* mApp_)
{
	mApp = mApp_;
	interfGeom = mApp->worker.GetGeometry();
}

void ImWindow::Show()
{
	drawn = true;
	if (txtW == 0 || txtH == 0) {
		txtW = ImGui::CalcTextSize("0").x;
		txtH = ImGui::GetTextLineHeightWithSpacing();
	}
}

void ImWindow::Hide()
{
	drawn = false;
}

void ImWindow::Toggle()
{
	if (drawn) Hide();
	else Show();
}

const bool ImWindow::IsVisible() {
	return drawn;
}

void ImWindow::SetVisible(bool value)
{
	drawn = value;
}

bool ImUtils::ParseExpression(const std::string& expression, GLFormula& formula)
{
	if (expression.empty())
	{
		return false;
	}
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

void ImUtils::ComputeManualExpression(bool& drawManual, GLFormula& formula, std::vector<double>& xVals, std::vector<double>& yVals, size_t count)
{
	if (!drawManual) return;

	auto xVariable = formula.GetVariableAt(0);
	for (double i = 0; i < count; i++) {
		xVariable->value = static_cast<double>(i);
		double y;
		try {
			y = formula.Evaluate();
		}
		catch (...) {
			//if (formula.hasEvalError) std::cout << formula.evalErrorMsg << std::endl;
			formula.hasEvalError = false;
			continue; //Eval. error, but maybe for other x it is possible to evaluate (ex. div by zero)
		}
		yVals.push_back(y);
		xVals.push_back(i);
	}
	if (xVals.size() == 0) {
		ImIOWrappers::InfoPopup("Error", "Error evaluating formula");
		drawManual = false;
	}
}

long long ImUtils::EntryIndexFromXAxisValue(double xAxisValue, const ImPlotData& plot)
{
	std::vector<double> data = *plot.x;
	return EntryIndexFromXAxisValue(xAxisValue, data);
}

long long ImUtils::EntryIndexFromXAxisValue(double xAxisValue, const std::vector<double>& vector)
{
	if (vector.size() == 0) return -1;
	if (xAxisValue < vector[0]) return -1;
	if (xAxisValue > vector[vector.size()-1]) return -1;
	double bestMatchDiff = abs(xAxisValue - vector[0]);
	long long idxOfBestMatch = 0;
	for (size_t i = 0; i < vector.size(); i++) {
		if (abs(xAxisValue - vector[i]) < bestMatchDiff) {
			bestMatchDiff = abs(xAxisValue - vector[i]);
			idxOfBestMatch = i;
		}
	}

	return idxOfBestMatch;
}

ImPlotData ImUtils::MakePlotData(size_t id, std::shared_ptr<std::vector<double>> x, std::shared_ptr<std::vector<double>> y, ImVec4 color)
{
	ImPlotData out = ImPlotData();
	out.id = id;
	out.x = x;
	out.y = y;
	out.color = color;
	return out;
}

void ImUtils::DrawValueOnHover(const std::vector<ImPlotData>& data, bool drawManual, const std::vector<double>* manualxValues, const std::vector<double>* manualyValues) {
	if (ImPlot::IsPlotHovered()) {
		ImPlotPoint mouse = ImPlot::GetPlotMousePos();
		if (data.size() != 0 || drawManual) {
			int entryIdx = -1;
			double minYDiff = ImPlot::PlotToPixels(ImPlotPoint(0, ImPlot::GetPlotLimits().Y.Size())).y * 10;
			int plotIdx = -2;
			for (int i = 0; i < data.size(); i++) { // find which plot contains the value closest to cursor
				int entryIdxTmp = ImUtils::EntryIndexFromXAxisValue(mouse.x, data[i]);
				double dist;
				if (entryIdxTmp != -1) dist = abs(ImPlot::PlotToPixels(ImPlotPoint(0, mouse.y)).y - ImPlot::PlotToPixels(ImPlotPoint(0, data[i].y->at(entryIdxTmp))).y);
				if (entryIdxTmp != -1 && dist < minYDiff) {
					minYDiff = dist;
					plotIdx = i;
					entryIdx = entryIdxTmp;
				}
			}
			if (drawManual) {
				int entryIdxTmp = ImUtils::EntryIndexFromXAxisValue(mouse.x, *manualxValues);
				double dist = minYDiff;
				if (entryIdxTmp != -1) dist = abs(ImPlot::PlotToPixels(ImPlotPoint(0, mouse.y)).y - ImPlot::PlotToPixels(ImPlotPoint(0, manualyValues->at(entryIdxTmp))).y);
				if (entryIdxTmp != -1 && dist < minYDiff) {
					minYDiff = dist;
					plotIdx = -1;
					entryIdx = entryIdxTmp;
				}
			}
			if (plotIdx == -1) {
				entryIdx = ImUtils::EntryIndexFromXAxisValue(mouse.x, *manualxValues);
			}
			else if (plotIdx >= 0 && plotIdx < data.size()) {
				entryIdx = ImUtils::EntryIndexFromXAxisValue(mouse.x, data[plotIdx]);
			}
			if ((plotIdx == -1 && entryIdx != -1) || (plotIdx >= 0 && plotIdx < data.size() && entryIdx >= 0 && entryIdx < data[plotIdx].x->size())) {
				double X = plotIdx == -1 ? manualxValues->at(entryIdx) : data[plotIdx].x->at(entryIdx);
				double Y = plotIdx == -1 ? manualyValues->at(entryIdx) : data[plotIdx].y->at(entryIdx);
				ImPlot::PushStyleColor(0, ImVec4(0, 0, 0, 1));
				ImPlot::PlotScatter("", &X, &Y, 1);
				ImPlot::PopStyleColor();

				std::string xVal, yVal;
				xVal = fmt::format("{:.4}", X);
				yVal = fmt::format("{:.4}", Y);

				ImVec2 tooltipPos = ImPlot::PlotToPixels(ImPlotPoint(X, Y));
				float txtW = ImGui::CalcTextSize("X").x;
				float txtH = ImGui::GetTextLineHeightWithSpacing();
				if (ImPlot::GetPlotPos().y + ImPlot::GetPlotSize().y - tooltipPos.y < 2 * txtH) tooltipPos.y -= 2.5 * txtH;
				if (ImPlot::GetPlotPos().x + ImPlot::GetPlotSize().x - tooltipPos.x < 20 * txtW) tooltipPos.x -= (std::max(xVal.length(), yVal.length()) + 3.5) * txtW;

				ImGui::SetNextWindowPos(tooltipPos);
				ImGui::BeginTooltipEx(0, 0);
				ImGui::Text("X=" + xVal + "\nY=" + yVal);
				ImGui::EndTooltip();
			}
		}
	}
}