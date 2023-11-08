#pragma once
#include <string>
#include <vector>
#include "GLFormula.h"
#include "Geometry_shared.h"
#include "imgui.h"
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
	bool drawn = false;
	float txtW=0, txtH=0;
	Interface* mApp;
	InterfaceGeometry* interfGeom;
};

// functions used by some(>1) ImWindows but not common enough (==2) to be member functions
namespace ImUtils {
	bool ParseExpression(const std::string& expression, GLFormula& formula);
	void ComputeManualExpression(bool& drawManual, GLFormula& formula, std::vector<double>& xVals, std::vector<double>& yVals, size_t count);
}

typedef struct {
	size_t facetID;
	std::shared_ptr<std::vector<double>> x;
	std::shared_ptr<std::vector<double>> y;
	ImVec4 color;
} ImPlotData;