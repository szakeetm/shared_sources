#pragma once
#include "ImguiWindowBase.h"
#include "Interface.h"
#include <string>
#include <memory>
#include <imgui.h>

class ImFormulaEditor : public ImWindow {
public:
	void Draw();
	void Init(Interface* mApp_, std::shared_ptr<Formulas> formulas_);
protected:
	void DrawFormulaList();
	Interface* mApp=0;
	std::shared_ptr<Formulas> appFormulas;
	int selRow = -1;

	class ImFormattingHelp : public ImWindow {
	public:
		void Draw();
	};
	int formulasSize;
	enum direction {up,down};
	void Move(direction d);
	ImFormattingHelp help;
	ImGuiIO* io;
	bool blue = false;
	std::string ExportCurrentFormulas();
	std::string ExportFormulasAtAllMoments();
	std::string GetFormulaValue(int index);
};