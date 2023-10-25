#include "ImguiFormulaEditor.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "ImguiMenu.h"
#include <sstream>


#ifdef MOLFLOW
#include "../../src/MolFlow.h"
#endif

extern char formulaSyntax[];

// remove when ImGui Become main UI
#include "FormulaEditor.h"
void UpdateLegacyGUI() {
	// the following code is just to keep legacy formula editor up to date, can be removed when ImGui becomes
	// the main UI
	if (mApp->formulaEditor) {
		mApp->formulaEditor->RebuildList();
		mApp->formulaEditor->Refresh();
		mApp->formulaEditor->UpdateValues();
	}
}

void ImFormulaEditor::DrawFormulaList() {
	static std::string newExpression, newName, changeExpression, changeName;
	static int changeIndex = -1;
	float columnW;
	formulasSize = appFormulas->formulas.size();
	if (ImGui::BeginTable("##FL", 5, ImGuiTableFlags_SizingStretchSame)) {

		ImGui::TableSetupColumn("##ID",ImGuiTableColumnFlags_WidthFixed,txtW*4);
		ImGui::TableSetupColumn("Expression");
		ImGui::TableSetupColumn("Name (Optional)");
		ImGui::TableSetupColumn("Value");
		ImGui::TableSetupColumn("Action",ImGuiTableColumnFlags_WidthFixed,txtW*6);

		ImGui::TableHeadersRow();
		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH*2));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // Optional: No padding between cells
		for (int i = 0; i < formulasSize; i++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(std::to_string(i + 1).c_str(), selRow==i, selRow == i ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_SpanAllColumns)) {
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
				columnW = ImGui::GetContentRegionAvailWidth();
				ImGui::SetNextItemWidth(columnW);
				ImGui::InputText("##changeExp", &changeExpression);
				ImGui::TableSetColumnIndex(2);
				ImGui::SetNextItemWidth(columnW);
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
			bool isDiff = changeExpression != appFormulas->formulas[i].GetExpression() || changeName != appFormulas->formulas[i].GetName();
			if (selRow == i && isDiff && (ImGui::Button("Apply")	|| io->KeysDown[SDL_SCANCODE_RETURN]
																	|| io->KeysDown[SDL_SCANCODE_RETURN2])) {
				changeIndex = i;
			}

		}

		ImGui::TableNextRow(); // empty row at the end
		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable((std::to_string(formulasSize + 1)).c_str(), selRow == formulasSize)) {
			if (selRow == formulasSize) selRow = -1;
			else selRow = formulasSize;
		}
		ImGui::TableSetColumnIndex(1);
		columnW = ImGui::GetContentRegionAvailWidth();
		ImGui::SetNextItemWidth(columnW);
		if (ImGui::InputText("##NE", &newExpression)) {
			selRow = formulasSize;
		}
		ImGui::TableSetColumnIndex(2);
		ImGui::SetNextItemWidth(columnW);
		if (ImGui::InputText("##NN", &newName)) {
			selRow = formulasSize;
		}
		ImGui::TableSetColumnIndex(4);
		if (!(newName == "" && newExpression == "") && (ImGui::Button("Add")
			|| (selRow == formulasSize && (io->KeysDown[SDL_SCANCODE_RETURN]
										|| io->KeysDown[SDL_SCANCODE_RETURN2])))) {
			appFormulas->AddFormula(newName, newExpression);
			formulasSize = appFormulas->formulas.size();
			appFormulas->formulas[formulasSize-1].Parse();
			appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
			selRow = formulasSize-1;
			changeIndex = -1;
			changeName = newName;
			changeExpression = newExpression;
			newName = "";
			newExpression = "";
			UpdateLegacyGUI();
		}


		// apply changes
		if (changeIndex > -1) {
			if (changeName == "" && changeExpression == "") {
				//delete formula
				appFormulas->formulas.erase(appFormulas->formulas.begin() + selRow);
			}
			else {
				//modify formula
				appFormulas->formulas[changeIndex].SetName(changeName);
				appFormulas->formulas[changeIndex].SetExpression(changeExpression);
				appFormulas->formulas[changeIndex].Parse();
			}
			appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
			changeIndex = -1;
			selRow = -1;

			UpdateLegacyGUI();
		}

		ImGui::EndTable();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		// child will fill the space below table, its only purpose is to be clicked in order to deselect rows
		// deselecting rows no longer 'submits' changes, that was bad design in my opinion ~Tym Mro
		ImGui::BeginChild("##clickBait",ImVec2(0,0),false,ImGuiWindowFlags_NoScrollbar);
		if (ImGui::IsWindowHovered() && io->MouseDown[0]) {
			selRow = -1;
		}
		ImGui::EndChild();
	}
}

void ImFormulaEditor::Move(direction d)
{
	if (d == up && selRow > 0 && selRow < formulasSize) {
		std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow - 1]);
		std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow - 1]);
		std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow - 1]);
		--selRow;
	}
	else if (d == down && selRow >= 0 && selRow < formulasSize - 1) {
		std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow + 1]);
		std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow + 1]);
		std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow + 1]);
		++selRow;
	}
}


void ImFormulaEditor::Draw() {
	if (!drawn) return;
	formulasSize = appFormulas->formulas.size();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 55, txtH*10), ImVec2(txtW*100,txtH*100));
	ImGui::SetNextWindowSize(ImVec2(txtW * 75, txtH * 20), ImGuiCond_FirstUseEver);
	ImGui::Begin("Formula editor", &drawn, ImGuiWindowFlags_NoSavedSettings);
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
	
	bool moveUpEnable = selRow > 0 && selRow < formulasSize;
	if (!moveUpEnable) ImGui::BeginDisabled();
	if (ImGui::Button("Move Up")) {
		Move(up);
	}
	if (!moveUpEnable) ImGui::EndDisabled();
	
	ImGui::SameLine();
	bool moveDownEnable = (selRow >= 0 && selRow < formulasSize - 1);
	if (!moveDownEnable) ImGui::BeginDisabled();
	if (ImGui::Button("Move Down")) {
		Move(down);
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
