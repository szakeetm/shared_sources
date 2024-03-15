#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "ImguiFormulaEditor.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "ImguiMenu.h"
#include "ImguiExtensions.h"
#include "ImguiConvergencePlotter.h"
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
	// check if user selected a different moment
	bool momentChanged = lastMoment != mApp->worker.displayedMoment;
	if (momentChanged) lastMoment = mApp->worker.displayedMoment;
	// if auto update is enabled and moment changed recalculate values
	if (momentChanged && mApp->autoUpdateFormulas) {
		appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
	}

	float columnW;
	blue = mApp->worker.displayedMoment != 0; // control wether to color values blue or not
	formulasSize = static_cast<int>(appFormulas->formulas.size()); // very frequently used value so it is stored
	if (ImGui::BeginTable("##FL", 5, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable)) {
		// Headers
		ImGui::TableSetupColumn("##ID",ImGuiTableColumnFlags_WidthFixed,txtW*4);
		ImGui::TableSetupColumn("Expression");
		ImGui::TableSetupColumn("Name (Optional)");
		ImGui::TableSetupColumn("Value");
		ImGui::TableSetupColumn("Action",ImGuiTableColumnFlags_WidthFixed,txtW*6);

		ImGui::TableHeadersRow();
		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, txtH*0.1));  // Adjusts row height
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));     // No padding between cells

		// this loop draws a row
		for (int i = 0; i < formulasSize; i++) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);

			// handle selecting a row
			if (ImGui::Selectable(std::to_string(i + 1).c_str(), selRow==i, selRow == i ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_SpanAllColumns)) {
				if (selRow == i) selRow = -1;
				else {
					selRow = i;
					changeExpression = appFormulas->formulas[i].GetExpression();
					changeName = appFormulas->formulas[i].GetName();
				}
			}
			ImGui::TableSetColumnIndex(1);
			if(selRow != i) // unselected - just draw values
			{
				ImGui::Text(appFormulas->formulas[i].GetExpression());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(appFormulas->formulas[i].GetName());
			}
			else { // selected - draw inputs
				columnW = ImGui::GetContentRegionAvail().x;
				ImGui::SetNextItemWidth(columnW);
				ImGui::InputText("##changeExp", &changeExpression);
				ImGui::TableSetColumnIndex(2);
				columnW = ImGui::GetContentRegionAvail().x;
				ImGui::SetNextItemWidth(columnW);
				ImGui::InputText("##changeNam", &changeName);
			}
			// values column
			ImGui::TableSetColumnIndex(3);
			while (valuesBuffer.size() <= i) valuesBuffer.push_back("");
			if (mApp->autoUpdateFormulas) {
				valuesBuffer[i] = mApp->appFormulas->GetFormulaValue(i);
			}
			ImGui::TextColored(ImVec4(0.f, 0.f, blue?1.f:0.f, 1.f), valuesBuffer[i].c_str());
			ImGui::TableSetColumnIndex(4);
			// check if value changed
			bool isDiff = changeExpression != appFormulas->formulas[i].GetExpression() || changeName != appFormulas->formulas[i].GetName();
			// show button only if the row is selected and there was a change
			if (selRow == i && isDiff && (ImGui::Button(" Apply ")	|| io->KeysDown[ImGuiKey_Enter]
																	|| io->KeysDown[SDL_SCANCODE_KP_ENTER])) {
				changeIndex = i; // set it when button or enter is pressed
			}

		}

		ImGui::TableNextRow(); // empty row at the end for new formulas
		ImGui::TableSetColumnIndex(0);
		if (ImGui::Selectable((std::to_string(formulasSize + 1)).c_str(), selRow == formulasSize)) {
			if (selRow == formulasSize) selRow = -1;
			else selRow = formulasSize;
		}
		// inputs for expression and name
		ImGui::TableSetColumnIndex(1);
		columnW = ImGui::GetContentRegionAvail().x;
		ImGui::SetNextItemWidth(columnW);
		if (ImGui::InputText("##NE", &newExpression)) {
			selRow = formulasSize; // auto select row if field changed
		}
		ImGui::TableSetColumnIndex(2);
		columnW = ImGui::GetContentRegionAvail().x;
		ImGui::SetNextItemWidth(columnW);
		if (ImGui::InputText("##NN", &newName)) {
			selRow = formulasSize;
		}
		ImGui::TableSetColumnIndex(4);
		// show button if either field is not empty
		if (!(newName == "" && newExpression == "") && (ImGui::Button(" Add ")
			|| (selRow == formulasSize && (io->KeysDown[ImGuiKey_Enter]
										|| io->KeysDown[SDL_SCANCODE_KP_ENTER])))) {
			// add new formula when button is pressed or row is selected and enter is pressed
			appFormulas->AddFormula(newName, newExpression);
			formulasSize = static_cast<int>(appFormulas->formulas.size());
			appFormulas->formulas[formulasSize-1].Parse();
			if (mApp->autoUpdateFormulas) { // formula was added, if autoUpdate is true update all values
				appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
			}
			selRow = formulasSize-1; // select newly added formula
			// reset state
			changeIndex = -1;
			changeName = newName;
			changeExpression = newExpression;
			newName = "";
			newExpression = "";
			// remove when ImGui becomes main UI
			UpdateLegacyGUI();
			mApp->imWnd->convPlot.Refresh();
		}


		// apply changes
		if (changeIndex > -1) {
			if (changeName == "" && changeExpression == "") {
				//delete formula
				mApp->imWnd->convPlot.RemovePlot(selRow); // prevent convergence plotter crash when removing plotted formula
				appFormulas->formulas.erase(appFormulas->formulas.begin() + selRow);
				valuesBuffer.erase(valuesBuffer.begin() + selRow);
				mApp->imWnd->convPlot.DecrementFormulaIndicies(selRow); // prevent convergence plotter crash when removing plotted formula
			}
			else {
				//modify formula
				mApp->imWnd->convPlot.RemovePlot(changeIndex); // undraw formula from conv plot on edit
				appFormulas->formulas[changeIndex].SetName(changeName);
				appFormulas->formulas[changeIndex].SetExpression(changeExpression);
				appFormulas->formulas[changeIndex].Parse();
			}
			if (mApp->autoUpdateFormulas) {
				appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
			}
			changeIndex = -1;
			selRow = -1;

			UpdateLegacyGUI();
			mApp->imWnd->convPlot.Refresh();
		}

		ImGui::EndTable();
		ImGui::OpenPopupOnItemClick("##FEFLcontext", ImGuiPopupFlags_MouseButtonRight);
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

// move selected formula up or down the list
void ImFormulaEditor::Move(direction d)
{
	if (d == up && selRow > 0 && selRow < formulasSize) {
		std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow - 1]);
		std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow - 1]);
		std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow - 1]);
		--selRow; // keep it selected
	}
	else if (d == down && selRow >= 0 && selRow < formulasSize - 1) {
		std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow + 1]);
		std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow + 1]);
		std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow + 1]);
		++selRow;
	}
}

// converts currently visible expressions names and values to tab separated values string
std::string ImFormulaEditor::ExportCurrentFormulas()
{
	return mApp->appFormulas->ExportCurrentFormulas();
}

// TODO: offload this to a thread and show progress bar
// dependng on a number of formulas and moments
// this could take a really long time
std::string ImFormulaEditor::ExportFormulasAtAllMoments() {
	LockWrapper imlock(mApp->imguiRenderLock);
	return mApp->appFormulas->ExportFormulasAtAllMoments(&mApp->worker);
}

void ImFormulaEditor::Draw() {
	if (!drawn) return;
	formulasSize = static_cast<int>(appFormulas->formulas.size());
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	// ImGui has no min size constraint so we set both min and max, with max absurdly high
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 55, txtH*10), ImVec2(txtW*1000,txtH*1000));
	ImGui::SetNextWindowSize(ImVec2(txtW * 75, txtH * 20), ImGuiCond_FirstUseEver);
	ImGui::Begin("Formula editor", &drawn, ImGuiWindowFlags_NoSavedSettings);

	float formulaListHeight = ImGui::GetWindowHeight() - txtH*8;
	ImGui::BeginChild("###Formula list",ImVec2(0, formulaListHeight),true,0);
	ImGui::Text("Formula list");
	ImGui::OpenPopupOnItemClick("##FEFLcontext", ImGuiPopupFlags_MouseButtonRight);

	DrawFormulaList();
	// context menu definition
	if (ImGui::BeginPopupContextItem("##FEFLcontext")) {
		std::string text;
		if (ImGui::Selectable("Copy table")) { 
			text = ExportCurrentFormulas();
			SDL_SetClipboardText(text.c_str()); 
		}
		if (ImGui::Selectable("Copy table (for all time moments)")) { 
			text = ExportFormulasAtAllMoments();
			SDL_SetClipboardText(text.c_str()); 
		}
		ImGui::EndPopup();
	}

	ImGui::EndChild();
	if (ImGui::Button("Recalculate now")) {
		appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
		while (valuesBuffer.size() <= appFormulas->formulas.size()) valuesBuffer.push_back("");
		for (int i = 0; i < appFormulas->formulas.size(); i++) {
			valuesBuffer[i] = mApp->appFormulas->GetFormulaValue(i);
		}
	}
	ImGui::SameLine();
	float dummyWidthA = ImGui::GetContentRegionAvail().x - txtW*20.5f;
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
	ImGui::Checkbox("Auto-update formulas", &mApp->autoUpdateFormulas);
	ImGui::SameLine();
	float dummyWidthB = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Open convergence plotter >>").x - 2 * txtW;
	ImGui::Dummy(ImVec2(dummyWidthB, txtH));
	ImGui::SameLine();
	if (ImGui::Button("Open convergence plotter >>")) {
		ImVec2 pos;
		pos.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x + txtW;
		pos.y = ImGui::GetWindowPos().y;
		ImGui::SetWindowPos("Convergence Plotter", pos);
		ImMenu::ConvergencePlotterMenuPress();
	}
	if (ImGui::Button("Syntax help")) {
		ImVec2 pos;
		pos.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x + txtW;
		pos.y = ImGui::GetWindowPos().y;
		ImGui::SetWindowPos("Formula Editor Syntax Help", pos);
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
	blue = mApp->worker.displayedMoment != 0;
}

void ImFormulaEditor::ImFormattingHelp::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW*80, txtH*10), ImVec2(txtW*100,txtH*35));
	ImGui::SetNextWindowSize(ImVec2(txtW*80, txtH*35),ImGuiCond_FirstUseEver);
	ImGui::Begin("Formula Editor Syntax Help", &drawn, ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("##FEHTXT", ImVec2(0,ImGui::GetContentRegionAvail().y-1.5f*txtH));
	ImGui::TextWrapped(formulaSyntax);
	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter("Close");
	if (ImGui::Button("Close")) this->Hide();
	ImGui::End();
}
