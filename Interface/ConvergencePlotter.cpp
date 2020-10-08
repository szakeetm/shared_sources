/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY
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
#include "ConvergencePlotter.h"
#include "GLApp/GLToolkit.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLTextField.h"
#include "GLApp/GLToggle.h"
#include "Helper/MathTools.h"
#include "GLApp/GLList.h"
#include "GLApp/GLChart/GLChart.h"
#include "GLApp/GLParser.h"

#include "Geometry_shared.h"
#include "Facet_shared.h"
#include <cmath>
#include <Helper/StringHelper.h>

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

constexpr size_t max_vector_size() {return 2048;};

/**
* \brief Constructor with initialisation for Convergence plotter window (Tools/Convergence Plotter)
*/
ConvergencePlotter::ConvergencePlotter(Worker *appWorker, std::vector<GLParser *> *formulaPtr,
                                       std::vector<std::vector<std::pair<size_t, double>>> *convValuesPtr)
        :GLWindow() , views{}{

	int wD = 625;
	int hD = 375;

	SetTitle("Convergence plotter");
	SetIconfiable(true);
	nbView = 0;
    worker = appWorker;
    formulas_vecPtr = formulaPtr;
    convValues_vecPtr = convValuesPtr;

	lastUpdate = 0.0f;

	chart = new GLChart(0);
	chart->SetBorder(BORDER_BEVEL_IN);
	chart->GetY1Axis()->SetGridVisible(true);
	chart->GetXAxis()->SetGridVisible(true);
	chart->GetY1Axis()->SetAutoScale(true);
	chart->GetY2Axis()->SetAutoScale(true);
	chart->GetY1Axis()->SetAnnotation(VALUE_ANNO);
	chart->GetXAxis()->SetAnnotation(VALUE_ANNO);
	Add(chart);

	dismissButton = new GLButton(0, "Dismiss");
	Add(dismissButton);

    pruneEveryNButton = new GLButton(0, "Remove every 4th");
    Add(pruneEveryNButton);
    pruneFirstNButton = new GLButton(0, "Remove first 100");
    Add(pruneFirstNButton);

	addButton = new GLButton(0, "Add curve");
	Add(addButton);

	removeButton = new GLButton(0, "Remove curve");
	Add(removeButton);

	removeAllButton = new GLButton(0, "Remove all");
	Add(removeAllButton);

	profCombo = new GLCombo(0);
	profCombo->SetEditable(true);
    Add(profCombo);

	logYToggle = new GLToggle(0, "Log Y");
	Add(logYToggle);

    colorToggle = new GLToggle(0, "Colorblind mode");
    Add(colorToggle);

    fixedLineWidthText = new GLLabel("Change linewidth:");
    Add(fixedLineWidthText);
    fixedLineWidthButton = new GLButton(0, "-> Apply linewidth");
    Add(fixedLineWidthButton);
    fixedLineWidthField = new GLTextField(0, "2");
    fixedLineWidthField->SetEditable(true);
    Add(fixedLineWidthField);

	formulaText = new GLTextField(0, "");
	formulaText->SetEditable(true);
	Add(formulaText);

	formulaBtn = new GLButton(0, "-> Plot expression");
	Add(formulaBtn);

	// Center dialog
	int wS, hS;
	GLToolkit::GetScreenSize(&wS, &hS);
	int xD = (wS - wD) / 2;
	int yD = (hS - hD) / 2;
	SetBounds(xD, yD, wD, hD);
	SetResizable(true);
	SetMinimumSize(wD, 220);

    RestoreDeviceObjects();

}

/**
* \brief Sets positions and sizes of all UI elements
* \param x x-coordinate of the element
* \param y y-coordinate of the element
* \param w width of the element
* \param h height of the element
*/
void ConvergencePlotter::SetBounds(int x, int y, int w, int h) {
	
	chart->SetBounds(7, 5, w - 15, h - 110);

    size_t lineHeightDiff = 45;
    formulaText->SetBounds(7, h - lineHeightDiff, 350, 19);
	formulaBtn->SetBounds(360, h - lineHeightDiff, 120, 19);;
	dismissButton->SetBounds(w - 100, h - lineHeightDiff, 90, 19);

    lineHeightDiff += 25;
    colorToggle->SetBounds(7, h - lineHeightDiff, 105, 19);
    fixedLineWidthText->SetBounds(112, h - lineHeightDiff, 93, 19);
    fixedLineWidthField->SetBounds(206, h - lineHeightDiff, 30, 19);
    fixedLineWidthButton->SetBounds(240, h - lineHeightDiff, 100, 19);
    pruneEveryNButton->SetBounds(w-215, h - lineHeightDiff, 100, 19);
    pruneFirstNButton->SetBounds(w-110, h - lineHeightDiff, 100, 19);

    lineHeightDiff += 25;
    profCombo->SetBounds(7, h - lineHeightDiff, 160, 19);
    logYToggle->SetBounds(180, h - lineHeightDiff, 40, 19);
    addButton->SetBounds(w - 270, h - lineHeightDiff, 80, 19);
    removeButton->SetBounds(w - 180, h - lineHeightDiff, 80, 19);
    removeAllButton->SetBounds(w - 90, h - lineHeightDiff, 80, 19);

    GLWindow::SetBounds(x, y, w, h);

}

/**
* \brief Resizes Vector storing convergence values if needed (change of expressions, etc.)
*/
void ConvergencePlotter::UpdateVector() {
    //Rebuild vector size
    size_t nbFormulas = formulas_vecPtr->size();
    if(convValues_vecPtr->size() != nbFormulas) {
        convValues_vecPtr->clear();
        convValues_vecPtr->resize(nbFormulas);
    }
}

/**
* \brief Resets convergence data e.g. when simulation resets
*/
void ConvergencePlotter::ResetData() {
    //Rebuild vector size
    for(auto& convVec : *convValues_vecPtr){
        convVec.clear();
    }
}

/**
* \brief Removes every everyN-th element from the convergence vector in case the max size has been reached
*/
void ConvergencePlotter::pruneEveryN(size_t everyN) {
    for(auto& convVec : *convValues_vecPtr){
        for (int i = convVec.size()-everyN; i > 0; i=i-everyN)
            convVec.erase(convVec.begin()+i);
    }
}

/**
* \brief Removes first n elements from the convergence vector
*/
void ConvergencePlotter::pruneFirstN(size_t n) {
    for(auto& convVec : *convValues_vecPtr){
            convVec.erase(convVec.begin(),convVec.begin()+std::min(n,convVec.size()));
    }
}

/**
* \brief Refreshes all window components (combo, chart, values)
*/
void ConvergencePlotter::Refresh() {

	//Rebuild selection combo box
	size_t nbFormulas = formulas_vecPtr->size(); // minimum 1 for custom input
	profCombo->Clear();
	if (nbFormulas) profCombo->SetSize(nbFormulas);

    for (size_t i = 0; i < nbFormulas; i++) {
        char tmp[128];
        sprintf(tmp, "[%zd] %s", i + 1, (*formulas_vecPtr)[i]->GetExpression());
        profCombo->SetValueAt(i, tmp, (int)i);
	}
	profCombo->SetSelectedIndex(nbFormulas-1 ? 0 : -1);

	//Remove profiles that aren't present anymore
	for (int v = nbView - 1; v >= 0; v--) { //int because it can be -1, nbView is also int
		if (views[v]->userData1 >= formulas_vecPtr->size()) {
			chart->GetY1Axis()->RemoveDataView(views[v]);
			SAFE_DELETE(views[v]);
			for (size_t j = v; j < nbView - 1; j++) views[j] = views[j + 1];
			nbView--;
		}
	}

	//Update values
	refreshViews();
}


/**
* \brief Displays window with refreshed values
* \param w worker handle
*/
void ConvergencePlotter::Display(Worker *w) {

	
    SetWorker(w);
    UpdateVector();
	Refresh();
	SetVisible(true);

}

/**
* \brief Refreshes the view if needed
* \param appTime current time of the application
*/
void ConvergencePlotter::Update(float appTime) {

	if (!formulas_vecPtr->empty()) {
	    UpdateVector();
        size_t lastNbDes = worker->globalHitCache.globalHits.hit.nbDesorbed;
        for(int formulaId = 0 ; formulaId < formulas_vecPtr->size();++formulaId){
            // TODO: Cross check integrity of formula with editor!?
            if((*convValues_vecPtr)[formulaId].size() >= max_vector_size()){
                pruneEveryN(4); // delete every 4th element for now
            }
            double r;
            (*formulas_vecPtr)[formulaId]->hasVariableEvalError = false;
            if ((*formulas_vecPtr)[formulaId]->Evaluate(&r)) {
                (*convValues_vecPtr)[formulaId].emplace_back(std::make_pair(lastNbDes,r));
            }
        }
		refreshViews();
		lastUpdate = appTime;
		return;
	}
	else if ((appTime - lastUpdate > 1.0f) && nbView) {
		if (worker->isRunning) refreshViews();
		lastUpdate = appTime;
	}

}

/**
* \brief Creates a plot from the expression given in the textbox of the form f(x)=EXPRESSION (e.g. 2*x+50)
*/
void ConvergencePlotter::plot() {

	GLParser *parser = new GLParser();
	parser->SetExpression(formulaText->GetText().c_str());
	if (!parser->Parse()) {
		GLMessageBox::Display(parser->GetErrorMsg(), "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}

	int nbVar = parser->GetNbVariable();
	if (nbVar == 0) {
		GLMessageBox::Display("Variable 'x' not found", "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}
	if (nbVar > 1) {
		GLMessageBox::Display("Too much variables or unknown constant", "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}
	VLIST *var = parser->GetVariableAt(0);
	if (!iequals(var->name, "x")) {
		GLMessageBox::Display("Variable 'x' not found", "Error", GLDLG_OK, GLDLG_ICONERROR);
		SAFE_DELETE(parser);
		return;
	}

	GLDataView *v;

	// Check that view is not already added
	bool found = false;
	int i = 0;
	while (i < nbView && !found) {
		found = (views[i]->userData1 == -1);
		if (!found) i++;
	}

	if (found) {
		v = views[i];
		v->SetName(formulaText->GetText().c_str());
		v->Reset();
	}
	else {

		if (nbView < 50) {
			v = new GLDataView();
			v->SetName(formulaText->GetText().c_str());
			v->userData1 = -1;
			chart->GetY1Axis()->AddDataView(v);
			views[nbView] = v;
			nbView++;
		}
		else {
			return;
		}
	}

	// Plot
	for (i = 0; i < 1000; i++) {
		double x = (double)i;
		double y;
		var->value = x;
		parser->Evaluate(&y);
		v->Add(x, y, false);
	}
	v->CommitChange();

	delete parser;

}

/**
* \brief Refreshes view by updating the data for the plot
*/
void ConvergencePlotter::refreshViews() {

	// Lock during update

	for (int i = 0; i < nbView; i++) {

		GLDataView *v = views[i];
		if (v->userData1 >= 0 && v->userData1 < convValues_vecPtr->size()) {
			v->Reset();

			if (worker->globalHitCache.globalHits.hit.nbDesorbed > 0){
                for (int j = 0; j < (*convValues_vecPtr)[v->userData1].size(); j++)
                    v->Add((*convValues_vecPtr)[v->userData1][j].first, (*convValues_vecPtr)[v->userData1][j].second, false);
			}
			v->CommitChange();
		}
	}
}

/**
* \brief Adds view/plot for a specific facet
* \param formulaId specific facet ID
* \return 0 if okay, 1 if already plotted
*/
int ConvergencePlotter::addView(int formulaId) {

	char tmp[128];

	// Check that view is not already added
	bool found = false;
	int i = 0;
	while (i < nbView && !found) {
		found = (views[i]->userData1 == formulaId);
		if (!found) i++;
	}
	if (found) {
		return 1;
	}
	if (nbView < MAX_VIEWS) {
		GLDataView *v = new GLDataView();
		sprintf(tmp, "%s", (*formulas_vecPtr)[formulaId]->GetExpression());
		v->SetName(tmp);
		//Look for first available color
		GLColor col = chart->GetFirstAvailableColor();
        int lineStyle = chart->GetFirstAvailableLinestyle(col);
		v->SetColor(col);
		v->SetMarkerColor(col);
		v->SetStyle(lineStyle);
		v->SetLineWidth(2);
		v->userData1 = formulaId;

		chart->GetY1Axis()->AddDataView(v);
		views[nbView] = v;
		nbView++;
    }

	return 0;
}

/**
* \brief Removes view/plot for a specific facet
* \param formulaId specific facet ID
* \return 0 if okay, 1 if not plotted
*/
int ConvergencePlotter::remView(int formulaId) {

	bool found = false;
	int i = 0;
	while (i < nbView && !found) {
		found = (views[i]->userData1 == formulaId);
		if (!found) i++;
	}
	if (!found) {
		return 1;
	}
	chart->GetY1Axis()->RemoveDataView(views[i]);
	SAFE_DELETE(views[i]);
	for (int j = i; j < nbView - 1; j++) views[j] = views[j + 1];
	nbView--;

    return 0;
}

/**
* \brief Resets the whole chart
*/
void ConvergencePlotter::Reset() {

	chart->GetY1Axis()->ClearDataView();
	for (int i = 0; i < nbView; i++) SAFE_DELETE(views[i]);
	nbView = 0;
}

/**
* \brief Function for processing various inputs (button, check boxes etc.)
* \param src Exact source of the call
* \param message Type of the source (button)
*/
void ConvergencePlotter::ProcessMessage(GLComponent *src, int message) {

	switch (message) {
	case MSG_BUTTON:
		if (src == dismissButton) {
			SetVisible(false);
		}
		else if (src == pruneEveryNButton) {
            pruneEveryN(4);
        }
		else if (src == pruneFirstNButton) {
            pruneFirstN(100);
        }
		else if (src == addButton) {
			int idx = profCombo->GetSelectedIndex();
			if (idx >= 0) { //Something selected (not -1)
                if(addView(profCombo->GetUserValueAt(idx)))
                    GLMessageBox::Display("Profile already plotted", "Info", GLDLG_OK, GLDLG_ICONINFO);
				refreshViews();
            }
		}
		else if (src == removeButton) {

			int idx = profCombo->GetSelectedIndex();
            if (idx >= 0) { //Something selected (not -1)
                if(remView(profCombo->GetUserValueAt(idx))){
                    GLMessageBox::Display("Profile not plotted", "Error", GLDLG_OK, GLDLG_ICONERROR);
                }
                refreshViews();
            }
		}
		else if (src == removeAllButton) {
			Reset();
		}
		else if (src == formulaBtn) {
			plot();
		}
		else if(src == fixedLineWidthButton) {
            int linW;
            fixedLineWidthField->GetNumberInt(&linW);
            for(int viewId = 0; viewId < nbView; viewId++){
                GLDataView *v = views[viewId];
                v->SetLineWidth(linW);
            }
        }
		break;
	case MSG_COMBO:
		if(src == profCombo){
            int profMode = profCombo->GetSelectedIndex();
        }
		break;
	case MSG_TOGGLE:
		if (src == logYToggle) {
			chart->GetY1Axis()->SetScale(logYToggle->GetState());
		}
		else if(src == colorToggle) {
		    if(!colorToggle->GetState())
                chart->SetColorSchemeDefault();
		    else
		        chart->SetColorSchemeColorblind();

		    const auto& colors = chart->GetColorScheme();
		    for(int viewId = 0; viewId < nbView; viewId++){
                GLDataView *v = views[viewId];
                auto col = colors[viewId%colors.size()];
                int lineStyle = chart->GetFirstAvailableLinestyle(col);
                v->SetColor(col);
                v->SetMarkerColor(col);
                v->SetStyle(lineStyle);

                std::string facId(v->GetName());
                facId = facId.substr(facId.find('#') + 1);
		    }
		}
		break;
	}
    //this->worker->GetGeometry()->SetPlottedFacets(plottedFacets);
	GLWindow::ProcessMessage(src, message);

}

/**
* \brief Adds views to the plotter if loaded form a file (XML)
* \param updatedViews vector containing the ids of the views
*/
void ConvergencePlotter::SetViews(const std::vector<int> &updatedViews) {
	Reset();
	for (int view : updatedViews)
		if (view< formulas_vecPtr->size())
			addView(view);
	//Refresh(); //Commented out: at this point, simulation results are not yet loaded
}

/**
* \brief Create and return a vector of view IDs
* \return vector containing the IDs of the views
*/
std::vector<int> ConvergencePlotter::GetViews() {
	std::vector<int>v;
	v.reserve(nbView);
	for (size_t i = 0; i < nbView; i++)
		v.push_back(views[i]->userData1);
	return v;
}

/**
* \brief Returns bool for active/inactive logarithmic scaling
* \return bool that expresses if Y axis is logarithmically scaled
*/
bool ConvergencePlotter::IsLogScaled() {
	return chart->GetY1Axis()->GetScale();
}

/**
* \brief Sets type of scale (logarithmic or not)
* \param bool that expresses if Y axis should be logarithmically scaled or not
*/
void ConvergencePlotter::SetLogScaled(bool logScale){
	chart->GetY1Axis()->SetScale(logScale);
	logYToggle->SetState(logScale);
}

/**
* \brief Sets worker handle for loading views before the full geometry
* \param w worker handle
*/
void ConvergencePlotter::SetWorker(Worker *w) { //for loading views before the full geometry

	worker = w;

}