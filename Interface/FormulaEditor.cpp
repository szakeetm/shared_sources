/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#include "FormulaEditor.h"
#include "ConvergencePlotter.h"
#include "Formulas.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "Helper/MathTools.h"
#include <sstream>
#include <algorithm>

#include "Worker.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
extern SynRad*mApp;
#endif


extern char formulaSyntax[];
extern int formulaSyntaxHeight;

static const size_t nbCol = 3;
static const char *flName[] = { "Expression","Name (optional)","Value" };
static const int   flAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };
static const int   fEdits[] = { EDIT_STRING,EDIT_STRING,0 };


FormulaEditor::FormulaEditor(Worker *w, std::shared_ptr<Formulas> formulas) : GLWindow() {
    columnRatios = { 0.333,0.333,0.333 };

    int wD = 600;
    int hD = 230; //Height extended runtime when formula syntax panel is expanded

    work = w;
    appFormulas = formulas;
    appFormulas->convergenceDataChanged = false;

    SetTitle("Formula Editor");
    SetIconfiable(true);

    SetResizable(true);

    panel1 = new GLTitledPanel("Formula list");
    Add(panel1);

    formulaList = new GLList(0);
    formulaList->SetColumnLabelVisible(true);
    formulaList->SetRowLabelVisible(true);
    formulaList->SetHScrollVisible(false);
    formulaList->SetGrid(true);
	formulaList->SetAutoRowLabel(true);
    Add(formulaList);

    recalcButton = new GLButton(0, "Recalculate now");
    Add(recalcButton);

    convPlotterButton = new GLButton(0, "Open convergence plotter >>");
    GLWindow::Add(convPlotterButton);

    sampleConvergenceTgl = new GLToggle(0, "Record values for convergence");
    sampleConvergenceTgl->SetState(true);
    appFormulas->recordConvergence = sampleConvergenceTgl->GetState();
    Add(sampleConvergenceTgl);

    moveUpButton = new GLButton(0, "Move Up");
    moveUpButton->SetEnabled(false);
    Add(moveUpButton);

    moveDownButton = new GLButton(0, "Move Down");
    moveDownButton->SetEnabled(false);
    Add(moveDownButton);

    panel2 = new GLTitledPanel("Format");
    panel2->SetClosable(true);
    panel2->Close();
    Add(panel2);

    descL = new GLLabel(formulaSyntax);
    descL->SetVisible(false); //Set visible runtime
    Add(descL);

    // Place in top left corner
    //int wS, hS;
    //GLToolkit::GetScreenSize(&wS, &hS);
    int xD = /*(wS - wD - 215)*/ 10;
    int yD = 30;
    SetBounds(xD, yD, wD, hD);

    RestoreDeviceObjects();

}

void FormulaEditor::ProcessMessage(GLComponent *src, int message) {
	switch (message) {
	case MSG_PANELR:
	{
		if (src == panel2) {
			int x, y, w, h;
			GetBounds(&x, &y, &w, &h);
			if (panel2->IsClosed()) {
				SetBounds(x, y, w, h - formulaSyntaxHeight);
				descL->SetVisible(false);
			}
			else {
				SetBounds(x, y, w, h + formulaSyntaxHeight);
				descL->SetVisible(true);
			}
		}
		break;
	}
	case MSG_BUTTON:
		if (src == recalcButton) {
            appFormulas->EvaluateFormulas(work->globalStatCache.globalHits.nbDesorbed);
            UpdateValues();
		}
		else if (src == convPlotterButton) {
            if (!mApp->convergencePlotter) {
                mApp->convergencePlotter = new ConvergencePlotter(work, mApp->appFormulas);
            }
            if(!mApp->convergencePlotter->IsVisible()){
                mApp->convergencePlotter->Refresh();
                mApp->convergencePlotter->SetVisible(true);
            }
            else {
                mApp->convergencePlotter->SetVisible(false);
            }
        }
		else if (src == moveUpButton) {
			int selRow = formulaList->GetSelectedRow();
			if (selRow <= 0) {
				//Interface bug
				DEBUG_BREAK;
				return;
			}
			std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow - 1]);
            std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow - 1]);
            std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow - 1]);
            formulaList->SetSelectedRow(selRow - 1);
			EnableDisableMoveButtons();
			Refresh();
		}
		else if (src == moveDownButton) {
			int selRow = formulaList->GetSelectedRow();
			if (selRow > appFormulas->formulas.size() - 2) {
				//Interface bug
				DEBUG_BREAK;
				return;
			}
			std::swap(appFormulas->formulas[selRow], appFormulas->formulas[selRow + 1]);
            std::swap(appFormulas->convergenceData[selRow], appFormulas->convergenceData[selRow + 1]);
            std::swap(appFormulas->formulaValueCache[selRow], appFormulas->formulaValueCache[selRow + 1]);
            formulaList->SetSelectedRow(selRow + 1);
			EnableDisableMoveButtons();
			Refresh();
		}
		break;
    case MSG_TOGGLE:
        if (src == sampleConvergenceTgl) {
            appFormulas->recordConvergence = sampleConvergenceTgl->GetState();
        }
        break;
	case MSG_TEXT:
	case MSG_LIST:
	{
		for (size_t row = 0; row < (formulaList->GetNbRow() - 1); row++) { //regular lines

			if (strcmp(formulaList->GetValueAt(0, row), userExpressions[row].c_str()) != 0) { //Expression changed
				if (row >= appFormulas->formulas.size()) {
					//Interface bug
					DEBUG_BREAK;
					return;
				}
				if (*(formulaList->GetValueAt(0, row)) != 0 || *(formulaList->GetValueAt(1, row)) != 0) //Name or expr. not empty
				{
                    appFormulas->formulas[row].SetExpression(formulaList->GetValueAt(0, row));
                    appFormulas->formulas[row].Parse();
					Refresh();
				}
				else
				{
                    appFormulas->formulas.erase(appFormulas->formulas.begin() + row);
                    appFormulas->UpdateVectorSize();
					Refresh();
				}
				EnableDisableMoveButtons();
				break;
			}

			if (strcmp(formulaList->GetValueAt(1, row), userFormulaNames[row].c_str()) != 0) { //Name changed
				if (*(formulaList->GetValueAt(0, row)) != 0 || *(formulaList->GetValueAt(1, row)) != 0) //Name or expr. not empty
				{
					appFormulas->formulas[row].SetName(formulaList->GetValueAt(1, row));
					Refresh();
				}
				else
				{
					appFormulas->formulas.erase(appFormulas->formulas.begin() + row);
                    appFormulas->UpdateVectorSize();
                    Refresh();
				}
				EnableDisableMoveButtons();
				break;
			}

		}
		if (formulaList->GetValueAt(0, formulaList->GetNbRow() - 1) != nullptr) { //last line
			if (*(formulaList->GetValueAt(0, formulaList->GetNbRow() - 1)) != 0) {
				//Add new line
				appFormulas->AddFormula("", formulaList->GetValueAt(0, formulaList->GetNbRow() - 1));
				Refresh();
			}
		}
		else if (formulaList->GetValueAt(1, formulaList->GetNbRow() - 1) != nullptr) { //last line
			if (*(formulaList->GetValueAt(1, formulaList->GetNbRow() - 1)) != 0) {
				//Add new line
                appFormulas->AddFormula("", formulaList->GetValueAt(1, formulaList->GetNbRow() - 1));
				Refresh();
			}
		}
		EnableDisableMoveButtons();
		appFormulas->convergenceDataChanged = true;

        appFormulas->EvaluateFormulas(work->globalStatCache.globalHits.nbDesorbed);
        UpdateValues();

		break;
	}
	case MSG_LIST_COL: {
        int x, y, w, h;
        GetBounds(&x, &y, &w, &h);
        auto sum = (double) (w - 45);
        std::vector<double> colWidths(nbCol);
        for (size_t i = 0; i < nbCol; i++) {
            colWidths[i] = (double) formulaList->GetColWidth(i);
        }
        for (size_t i = 0; i < nbCol; i++) {
            columnRatios[i] = colWidths[i] / sum;
        }
        break;
    }
    default:
        break;
	}

	GLWindow::ProcessMessage(src, message);
}

void FormulaEditor::SetBounds(int x, int y, int w, int h) {
	int formulaHeight = (panel2->IsClosed() ? 0 : formulaSyntaxHeight);
	panel1->SetBounds(5, 5, w - 10, h - 120 - formulaHeight);
	formulaList->SetBounds(10, 22, w - 20, h - 145 - formulaHeight);
	for (size_t i=0;i<nbCol;i++)
		formulaList->SetColumnWidth(i, (int)(columnRatios[i] * (double)(w - 45)));
	recalcButton->SetBounds(10, h - 110 - formulaHeight, 95, 20);

	moveUpButton->SetBounds(w - 150, h - 110 - formulaHeight, 65, 20);
	moveDownButton->SetBounds(w - 80, h - 110 - formulaHeight, 65, 20);

    sampleConvergenceTgl->SetBounds(10, h - 80 - formulaHeight, 200, 20);
	convPlotterButton->SetBounds(w-210, h - 80 - formulaHeight, 200, 20);
    
	panel2->SetBounds(5, h - 50 - formulaHeight, w - 10, 20 + formulaHeight); //Height will be extended runtime
	panel2->SetCompBounds(descL, 10, 15, w-30, formulaHeight);

	SetMinimumSize(400, 150 + formulaHeight);
	GLWindow::SetBounds(x, y, w, h);
}

void FormulaEditor::EnableDisableMoveButtons()
{
	int selRow = formulaList->GetSelectedRow(false);
	if (selRow == -1 || selRow >= ((int)(formulaList->GetNbRow()) - 1)) { //Nothing or very last (empty) row selected
		moveUpButton->SetEnabled(false);
		moveDownButton->SetEnabled(false);
	}
	else if (selRow == 0) { //First row
		moveUpButton->SetEnabled(false);
		moveDownButton->SetEnabled(true);
	}
	else if (selRow == ((int)(formulaList->GetNbRow()) - 2)) { //Last (formula) row
		moveUpButton->SetEnabled(true);
		moveDownButton->SetEnabled(false);
	}
	else {
		moveUpButton->SetEnabled(true);
		moveDownButton->SetEnabled(true);
	}
}

void FormulaEditor::RebuildList() {
	//Rebuild list based on locally stored userExpressions
	int x, y, w, h;
	GetBounds(&x, &y, &w, &h);
	formulaList->SetSize(nbCol, userExpressions.size() + 1);
	for (size_t i = 0; i<nbCol; i++)
		formulaList->SetColumnWidth(i, (int)(columnRatios[i] * (double)(w - 45)));
	formulaList->SetColumnLabels(flName);
	formulaList->SetColumnAligns((int *)flAligns);
	formulaList->SetColumnEditable((int *)fEdits);

	for (size_t u = 0; u < userExpressions.size(); u++) {
		formulaList->SetValueAt(0, u, userExpressions[u].c_str());
		formulaList->SetValueAt(1, u, userFormulaNames[u].c_str());
	}
}

void FormulaEditor::Refresh() {
	//Load contents of window from global (interface/app) formulas
	size_t nbFormula = appFormulas->formulas.size();
	userExpressions.resize(nbFormula);
	userFormulaNames.resize(nbFormula);
	for (size_t i = 0; i < nbFormula; i++) {
		userExpressions[i] = appFormulas->formulas[i].GetExpression();
		userFormulaNames[i] = appFormulas->formulas[i].GetName();
	}
	RebuildList();
    appFormulas->convergenceDataChanged = true;
    UpdateValues();
}

void FormulaEditor::UpdateValues() {

	// This only displays formula values already evaluated in appFormulas
	// Therefore formulas should be updated beforehand by calling appFormulas->EvaluateFormulas
	for (size_t i = 0; i < appFormulas->formulas.size(); i++) {
		if (appFormulas->formulas[i].hasParseError) {
			formulaList->SetValueAt(2, i, appFormulas->formulas[i].GetParseErrorMsg());
		}
		else if (appFormulas->formulas[i].hasEvalError) {
			formulaList->SetValueAt(2, i, appFormulas->formulas[i].GetEvalErrorMsg());
		} else {
			std::stringstream tmp;
			tmp << appFormulas->formulaValueCache[i].value; //not elegant but converts 12.100000000001 to 12.1 etc., fmt::format doesn't
			formulaList->SetValueAt(2, i, tmp.str());
#if defined(MOLFLOW)
			formulaList->SetColumnColor(2,work->displayedMoment == 0 ? COLOR_BLACK : COLOR_BLUE);
#endif
		}
	}
}