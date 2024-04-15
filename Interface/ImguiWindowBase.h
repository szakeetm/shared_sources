#pragma once
#include <string>
#include <vector>
#include "../GLApp/GLFormula.h"
#include "Geometry_shared.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS#include "imgui.h"
#include "imgui_internal.h"
#include <memory>

class Interface;

class ImWindow {
public:
	virtual void Draw() = 0;
	void Init(Interface* mApp_);
	void Show();
	void Hide();
	void Toggle();
	const bool IsVisible();
	void SetVisible(bool value);
protected:
	virtual void OnShow();
	bool drawn = false;
	float txtW=0, txtH=0;
	Interface* mApp=nullptr;
	InterfaceGeometry* interfGeom=nullptr;
};

typedef struct {
	size_t id;
	std::shared_ptr<std::vector<double>> x;
	std::shared_ptr<std::vector<double>> y;
	ImVec4 color;
} ImPlotData;

// functions used by some(>1) ImWindows but not common enough (==2) to be member functions
namespace ImUtils {
	bool ParseExpression(const std::string& expression, GLFormula& formula);
	void ComputeManualExpression(bool& drawManual, GLFormula& formula, std::vector<double>& xVals, std::vector<double>& yVals, double maxX, double start=0, double step=1);
	long long EntryIndexFromXAxisValue(double xAxisValue, const ImPlotData& plot);
	long long EntryIndexFromXAxisValue(double xAxisValue, const std::vector<double>& vector);
	ImPlotData MakePlotData(size_t id = 0, std::shared_ptr<std::vector<double>> x=std::make_shared<std::vector<double>>(), std::shared_ptr<std::vector<double>> y = std::make_shared<std::vector<double>>(), ImVec4 color = ImVec4(0,0,0,0));
	void DrawValueOnHover(const std::vector<ImPlotData>& data, bool drawManual = false, const std::vector<double>* manualxValues = 0, const std::vector<double>* manualyValues = 0);
}
