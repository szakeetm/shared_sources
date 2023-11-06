#include "ImguiWindowBase.h"
#include "imgui.h"
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
	xVals.clear();
	yVals.clear();
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
