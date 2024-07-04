#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include <memory>
#include "imgui.h"

class ImFormulaEditor : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_, std::shared_ptr<Formulas> formulas_);
	void Update();
protected:
	void OnShow() override;
	void DrawFormulaList();
	Interface* mApp=0;
	std::shared_ptr<Formulas> appFormulas;
	std::vector<std::string> valuesBuffer;

	// std::vector<bool> does not have a standardized implementation and does
	// not behave as expected, and behavior differs accross platforms
	// using a simple struct to wrap around the bool in order to control the
	// behavior
	struct boolWrapper { bool b = false; };
	std::vector<boolWrapper> changed;

	int selRow = -1;

	class ImFormattingHelp : public ImWindow {
	public:
		void Draw();
	};
	int formulasSize = 0;
	enum direction {up,down};
	void Move(direction d);
	ImFormattingHelp help;
	ImGuiIO* io = nullptr;
	bool blue = false;
	int lastMoment = 0;
	std::string ExportCurrentFormulas();
	std::string ExportFormulasAtAllMoments();	
};