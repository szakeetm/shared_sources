#include "FormulaEditor.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp\GLToggle.h"
#include "GLApp\GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "GLApp\MathTools.h"
#include <sstream>
#include <algorithm>

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

#ifdef SYNRAD
#include "SynRad.h"
#endif

extern GLApplication *theApp;
extern std::string formulaSyntax;

#ifdef MOLFLOW
extern MolFlow *mApp;
#endif

#ifdef SYNRAD
extern SynRad*mApp;
#endif

static const int   flWidth[] = { 200,200,200 };
static const char *flName[] = { "Expression","Name (optional)","Value" };
static const int   flAligns[] = { ALIGN_LEFT,ALIGN_LEFT,ALIGN_LEFT };
static const int   fEdits[] = { EDIT_STRING,EDIT_STRING,0 };

FormulaEditor::FormulaEditor(Worker *w) :GLWindow() {

	int wD = 650;
	int hD = 300; //Height extended runtime

	work = w;

	SetTitle("Formula Editor");
	SetIconfiable(true);

	panel1 = new GLTitledPanel("Formula list");
	panel1->SetBounds(5, 5, wD - 10, 250);
	Add(panel1);

	formulaList = new GLList(0);
	formulaList->SetBounds(10, 22, wD - 20, 200);
	formulaList->SetColumnLabelVisible(true);
	formulaList->SetRowLabelVisible(true);
	formulaList->SetHScrollVisible(false);
	formulaList->SetGrid(true);
	Add(formulaList);

	recalcButton = new GLButton(0, "Recalculate now");
	recalcButton->SetBounds(10, 229, 95, 20);
	Add(recalcButton);

	moveUpButton = new GLButton(0, "Move Up");
	moveUpButton->SetBounds(wD - 160, 229, 65, 20);
	moveUpButton->SetEnabled(false);
	Add(moveUpButton);

	moveDownButton = new GLButton(0, "Move Down");
	moveDownButton->SetBounds(wD - 90, 229, 65, 20);
	moveDownButton->SetEnabled(false);
	Add(moveDownButton);

	panel2 = new GLTitledPanel("Format");
	panel2->SetBounds(5, 260, wD - 10, 15); //Height will be extended runtime
	panel2->SetClosable(TRUE);
	panel2->Close();
	Add(panel2);

	descL = new GLLabel(formulaSyntax.c_str());
	descL->SetVisible(false); //Set visible runtime
	panel2->SetCompBounds(descL, 10, 15, wD - 30, 355);
	Add(descL);

	// Top right
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD - 215);
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
				SetBounds(x, y, w, h - 360);
				panel2->GetBounds(&x, &y, &w, &h);
				panel2->SetBounds(x, y, w, h - 360);
				/*descL->GetBounds(&x, &y, &w, &h);
				descL->SetBounds(x, y, w, h - 360);*/
				descL->SetVisible(false);
			}
			else {
				SetBounds(x, y, w, h + 360);
				panel2->GetBounds(&x, &y, &w, &h);
				panel2->SetBounds(x, y, w, h + 360);
				/*descL->GetBounds(&x, &y, &w, &h);
				descL->SetBounds(x, y, w, h - 360);*/
				descL->SetVisible(true);
			}
		}
		break;
	}
	case MSG_BUTTON:
		if (src == recalcButton) {
			ReEvaluate();
		}
		else if (src == moveUpButton) {
			int selRow = formulaList->GetSelectedRow();
			if (selRow <= 0) {
				//Interface bug
				__debugbreak();
				return;
			}
			std::swap(mApp->formulas_n[selRow], mApp->formulas_n[selRow - 1]);
			formulaList->SetSelectedRow(selRow - 1);
			EnableDisableMoveButtons();
			Refresh();
		}
		else if (src == moveDownButton) {
			int selRow = formulaList->GetSelectedRow();
			if (selRow > mApp->formulas_n.size() - 2) {
				//Interface bug
				__debugbreak();
				return;
			}
			std::swap(mApp->formulas_n[selRow], mApp->formulas_n[selRow + 1]);
			formulaList->SetSelectedRow(selRow + 1);
			EnableDisableMoveButtons();
			Refresh();
		}
		break;
	case MSG_TEXT:
	case MSG_LIST:
	{
		for (size_t row = 0; row < (formulaList->GetNbRow() - 1); row++) { //regular lines

			if (strcmp(formulaList->GetValueAt(0, row), userExpressions[row].c_str()) != 0) { //Expression changed
				if (!(row < mApp->formulas_n.size())) {
					//Interface bug
					__debugbreak();
					return;
				}
				if (*(formulaList->GetValueAt(0, row)) != 0 || *(formulaList->GetValueAt(1, row)) != 0) //Name or expr. not empty
				{
					mApp->formulas_n[row]->SetExpression(formulaList->GetValueAt(0, row));
					mApp->formulas_n[row]->Parse();
					Refresh();
				}
				else
				{
					SAFE_DELETE(mApp->formulas_n[row]);
					mApp->formulas_n.erase(mApp->formulas_n.begin() + row);
					Refresh();
				}
				EnableDisableMoveButtons();
				break;
			}

			if (strcmp(formulaList->GetValueAt(1, row), userFormulaNames[row].c_str()) != 0) { //Name changed
				if (*(formulaList->GetValueAt(0, row)) != 0 || *(formulaList->GetValueAt(1, row)) != 0) //Name or expr. not empty
				{
					mApp->formulas_n[row]->SetName(formulaList->GetValueAt(1, row));
					Refresh();
				}
				else
				{
					SAFE_DELETE(mApp->formulas_n[row]);
					mApp->formulas_n.erase(mApp->formulas_n.begin() + row);
					Refresh();
				}
				EnableDisableMoveButtons();
				break;
			}

		}
		if (formulaList->GetValueAt(0, formulaList->GetNbRow() - 1) != 0) { //last line
			if (*(formulaList->GetValueAt(0, formulaList->GetNbRow() - 1)) != 0) {
				//Add new line
				GLParser* newF = new GLParser();
				newF->SetExpression(formulaList->GetValueAt(0, formulaList->GetNbRow() - 1));
				newF->SetName("");
				newF->Parse();
				mApp->formulas_n.push_back(newF);
				Refresh();
			}
		}
		else if (formulaList->GetValueAt(1, formulaList->GetNbRow() - 1) != 0) { //last line
			if (*(formulaList->GetValueAt(1, formulaList->GetNbRow() - 1)) != 0) {
				//Add new line
				GLParser* newF = new GLParser();
				newF->SetExpression("");
				newF->SetName(formulaList->GetValueAt(1, formulaList->GetNbRow() - 1));
				newF->Parse();
				mApp->formulas_n.push_back(newF);
				Refresh();
			}
		}
		EnableDisableMoveButtons();
		break;
	}
	}

	GLWindow::ProcessMessage(src, message);
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

	formulaList->SetSize(3, userExpressions.size() + 1);
	formulaList->SetColumnWidths((int*)flWidth);
	formulaList->SetColumnLabels((char **)flName);
	formulaList->SetColumnAligns((int *)flAligns);
	formulaList->SetColumnEditable((int *)fEdits);

	size_t u; double latest = 0.0;

	for (u = 0; u < userExpressions.size(); u++) {
		formulaList->SetValueAt(0, u, userExpressions[u].c_str());
		formulaList->SetValueAt(1, u, userFormulaNames[u].c_str());
	}
}

void FormulaEditor::Refresh() {
	//Load contents of window from global (interface/app) formulas
	size_t nbFormula = mApp->formulas_n.size();
	userExpressions.resize(nbFormula);
	userFormulaNames.resize(nbFormula);
	for (size_t i = 0; i < nbFormula; i++) {
		userExpressions[i] = mApp->formulas_n[i]->GetExpression();
		userFormulaNames[i] = mApp->formulas_n[i]->GetName();
	}
	RebuildList();
	ReEvaluate();
}

void FormulaEditor::ReEvaluate() {
	//-----------------------------------------------------------------------
	//       NEW CODE
	//-----------------------------------------------------------------------

	for (size_t i = 0; i < mApp->formulas_n.size(); i++) {

		// Evaluate variables
		int nbVar = mApp->formulas_n[i]->GetNbVariable();
		bool ok = true;
		for (int j = 0; j < nbVar && ok; j++) {
			VLIST *v = mApp->formulas_n[i]->GetVariableAt(j);
			ok = mApp->EvaluateVariable(v);
			if (!ok) {
				std::stringstream tmp;
				tmp << "Invalid variable " << v->name;
				formulaList->SetValueAt(2, i, tmp.str().c_str());
			}
		}

		// Evaluation
		if (ok) { //Variables succesfully evaluated
			double r;
			mApp->formulas_n[i]->hasVariableEvalError = false;
			if (mApp->formulas_n[i]->Evaluate(&r)) {
				std::stringstream tmp;
				tmp << r;
				formulaList->SetValueAt(2, i, tmp.str().c_str());
			}
			else { //Variables OK but the formula itself can't be evaluated
				formulaList->SetValueAt(2, i, mApp->formulas_n[i]->GetErrorMsg());
			}
#ifdef MOLFLOW
			//formulas[i].value->SetTextColor(0.0f, 0.0f, worker.displayedMoment == 0 ? 0.0f : 1.0f);
			formulaList->SetColumnColor(2,mApp->worker.displayedMoment == 0 ? COLOR_BLACK : COLOR_BLUE);
#endif
		}
		else { //Error while evaluating variables
			   //formulas[i].value->SetText("Invalid variable name"); //We set it directly at the error location
		}
	}
}