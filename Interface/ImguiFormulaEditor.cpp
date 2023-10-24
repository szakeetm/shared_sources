#include "ImguiFormulaEditor.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "ImguiMenu.h"
#include <sstream>

#ifdef MOLFLOW
#include "../../src/Molflow.h"
#endif

extern char formulaSyntax[];

void ImFormulaEditor::DrawFormulaList() {
	static std::string newExpression, newName, changeExpression, changeName;
	static int changeIndex = -1;
	
	if (ImGui::BeginTable("##FL", 5, ImGuiTableFlags_SizingStretchSame)) {
		ImGui::TableSetupColumn("##ID",ImGuiTableColumnFlags_WidthFixed,txtW*4);

		ImGui::TableHeadersRow();
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("Expression");
		ImGui::TableSetColumnIndex(2);
		ImGui::Text("Name (Optional)");
		ImGui::TableSetColumnIndex(3);
		ImGui::Text("Value");
		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH*2));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // Optional: No padding between cells
		for (int i = 0; i < appFormulas->formulas.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(std::to_string(i + 1).c_str(), selRow==i, ImGuiSelectableFlags_None)) {
				if (selRow == i) selRow = -1;
				else {
					selRow = i;
					changeExpression = appFormulas->formulas[i].GetExpression();
					changeName = appFormulas->formulas[i].GetName();
				}
			}
			ImGui::TableSetColumnIndex(1);
			if(selRow != i)
			{
				ImGui::Text(appFormulas->formulas[i].GetExpression());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(appFormulas->formulas[i].GetName());
			}
			else {
				ImGui::InputText("##changeExp", &changeExpression);
				ImGui::TableSetColumnIndex(2);
				ImGui::InputText("##changeNam", &changeName);
			}
			ImGui::TableSetColumnIndex(3);
			if (appFormulas->formulas[i].hasParseError) {
				ImGui::Text(appFormulas->formulas[i].GetParseErrorMsg());
			}
			else if (appFormulas->formulas[i].hasEvalError) {
				ImGui::Text(appFormulas->formulas[i].GetEvalErrorMsg());
			}
			else {
				std::stringstream tmp; // ugly solution copied from legacy gui
				tmp << appFormulas->formulaValueCache[i].value; //not elegant but converts 12.100000000001 to 12.1 etc., fmt::format doesn't
				ImGui::Text(tmp.str());
			}
			ImGui::TableSetColumnIndex(4);
			if (selRow == i && ImGui::Button("Apply")) {
				changeIndex = i;
			}

		}

		ImGui::TableNextRow(); // empty row at the end
		ImGui::TableSetColumnIndex(0);
		ImGui::Text(std::to_string(appFormulas->formulas.size()+1));

		ImGui::TableSetColumnIndex(1);
		if (ImGui::InputText("##NE", &newExpression)) {
		}
		ImGui::TableSetColumnIndex(2);
		if (ImGui::InputText("##NN", &newName)) {
		}
		ImGui::TableSetColumnIndex(4);
		if (ImGui::Button("Add")) {
			appFormulas->AddFormula(newName, newExpression);
			appFormulas->formulas[appFormulas->formulas.size()-1].Parse();
			appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
			selRow = -1;
			newName = "";
			newExpression = "";
		}

		// apply changes
		if (changeIndex > -1) {
			appFormulas->formulas[changeIndex].SetName(changeName);
			appFormulas->formulas[changeIndex].SetExpression(changeExpression);
			appFormulas->formulas[changeIndex].Parse();
			appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
			changeIndex = -1;
			selRow = -1;
		}

		ImGui::EndTable();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
	}
}

void ImFormulaEditor::Draw() {
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 54, txtH*10), ImVec2(txtW*100,txtH*100));
	ImGui::Begin("Formula editor", &drawn);
	float formulaListHeight = ImGui::GetWindowHeight() - txtH*6;
	ImGui::BeginChild("Formula list",ImVec2(0, formulaListHeight),true,0);
	ImGui::Text("Formula list");

	DrawFormulaList();

	ImGui::EndChild();
	if (ImGui::Button("Recalculate now")) {
		appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
	}
	ImGui::SameLine();
	float dummyWidthA = ImGui::GetContentRegionAvailWidth() - txtW*20.5;
	ImGui::Dummy(ImVec2(dummyWidthA,txtH));
	ImGui::SameLine();
	
	bool moveUpEnable = selRow > 0;
	if (!moveUpEnable) ImGui::BeginDisabled();
	if (ImGui::Button("Move Up")) {
		std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow - 1]);
		std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow - 1]);
		std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow - 1]);
		--selRow;
	}
	if (!moveUpEnable) ImGui::EndDisabled();
	
	ImGui::SameLine();
	bool moveDownEnable = (selRow >= 0 && selRow < appFormulas->formulas.size() - 1);
	if (!moveDownEnable) ImGui::BeginDisabled();
	if (ImGui::Button("Move Down")) {
		std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow + 1]);
		std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow + 1]);
		std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow + 1]);
		++selRow;
	}
	if (!moveDownEnable) ImGui::EndDisabled();
	
	ImGui::Checkbox("Record values for convergence", &appFormulas->recordConvergence);
	ImGui::SameLine();
	float dummyWidthB = ImGui::GetContentRegionAvailWidth() - ImGui::CalcTextSize("Open convergence plotter >>").x - 2 * txtW;
	ImGui::Dummy(ImVec2(dummyWidthB, txtH));
	ImGui::SameLine();
	if (ImGui::Button("Open convergence plotter >>")) {
		ImMenu::ConvergencePlotterMenuPress();
	}
	if (ImGui::Button("Formatting help")) {
		help.Show();
	}
	ImGui::End();
	help.Draw();
}

void ImFormulaEditor::Init(Interface* mApp_, std::shared_ptr<Formulas> formulas_) {
	mApp = mApp_;
	appFormulas = formulas_;
	help = ImFormattingHelp();
	io = &ImGui::GetIO();
}

void ImFormulaEditor::ImFormattingHelp::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW*80, txtH*10), ImVec2(txtW*100,txtH*35));
	ImGui::SetNextWindowSize(ImVec2(txtW*80, txtH*35),ImGuiCond_FirstUseEver);
	ImGui::Begin("Formula Editor Help", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("##FEHTXT", ImVec2(0,ImGui::GetContentRegionAvail().y-1.5*txtH));
	ImGui::TextWrapped(formulaSyntax);
	ImGui::EndChild();
	if (ImGui::Button("Close")) this->Hide();
	ImGui::End();
}
