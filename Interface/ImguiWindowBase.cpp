#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "ImguiWindowBase.h"
#include "imgui.h"
//#include "imgui_internal.h"
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
	OnShow();
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
	if (value) Show();
	else Hide();
}

void ImWindow::OnShow()
{
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
	size_t nbVar = formula.GetNbVariable();
	if (nbVar == 0) {
		return true;
	}
	else if (nbVar > 1) {
		ImIOWrappers::InfoPopup("Error", "Too many variables or an unknown constant"); // Legacy had grammar issiues
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

void ImUtils::ComputeManualExpression(bool& drawManual, GLFormula& formula, std::vector<double>& xVals, std::vector<double>& yVals, double maxX, double start, double step)
{
	if (!drawManual) return;

	std::list<Variable>::iterator xVariable;
	if (formula.GetNbVariable() != 0) xVariable = formula.GetVariableAt(0);
	for (double i = start; i < maxX; i+=step) {
		if (formula.GetNbVariable() != 0) xVariable->value = static_cast<double>(i);
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
			int plotIdx = -2;
			int idxX = -1;
			double minDiff = abs(ImPlot::PlotToPixels(ImPlotPoint(0, ImPlot::GetPlotLimits().Y.Size())).y * 10);
			for (size_t p = 0; p < data.size(); p++) { // for every plot
				for (size_t i = 0; i < data[p].x->size(); i++) { // for every datapoint
					ImVec2 pltPoint = ImPlot::PlotToPixels(ImPlotPoint(data[p].x->at(i), data[p].y->at(i)));
					ImVec2 mousePos = ImGui::GetMousePos();
					double newDist = std::sqrt(std::pow(pltPoint.x - mousePos.x, 2) + std::pow(pltPoint.y - mousePos.y, 2));
					if (newDist < minDiff) {
						minDiff = newDist;
						plotIdx = static_cast<int>(p);
						idxX = static_cast<int>(i);
					}
				}
			}
			if (drawManual) {
				for (size_t i = 0; i < std::min(manualxValues->size(), manualyValues->size()); i++) { // for every datapoint
					ImVec2 pltPoint = ImPlot::PlotToPixels(ImPlotPoint(manualxValues->at(i), manualyValues->at(i)));
					ImVec2 mousePos = ImGui::GetMousePos();
					double newDist = std::sqrt(std::pow(pltPoint.x - mousePos.x, 2) + std::pow(pltPoint.y - mousePos.y, 2));
					if (newDist < minDiff) {
						minDiff = newDist;
						plotIdx = -1;
						idxX = static_cast<int>(i);
					}
				}
			}
			// do the actual drawing
			if ((plotIdx == -1 && idxX != -1) || (plotIdx >= 0 && plotIdx < data.size() && idxX >= 0 && idxX < data[plotIdx].x->size())) {
				double X = plotIdx == -1 ? manualxValues->at(idxX) : data[plotIdx].x->at(idxX);
				double Y = plotIdx == -1 ? manualyValues->at(idxX) : data[plotIdx].y->at(idxX);
				ImPlot::PushStyleColor(0, ImVec4(1.f, 0.5f, 0.5f, 0.8f));
				ImPlot::PlotScatter("", &X, &Y, 1);
				ImPlot::PopStyleColor();

				std::string xVal, yVal;
				xVal = fmt::format("{:.4}", X);
				yVal = fmt::format("{:.4}", Y);
				ImVec4 col(0.5f, 0.5f, 0.5f, 0.8f);

				ImVec2 tooltipPos = ImPlot::PlotToPixels(ImPlotPoint(X, Y));
				float txtW = ImGui::CalcTextSize("X").x;
				float txtH = ImGui::GetTextLineHeightWithSpacing();
				ImPlot::Annotation(X, Y, ImVec4(0,0,0,1), ImVec2(txtH, txtH), true, ("X=" + xVal + "\nY=" + yVal).c_str());
			}
		}
	}
}