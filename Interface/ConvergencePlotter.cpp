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
#include "GLApp/GLFormula.h"
#include "Formulas.h"
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

constexpr size_t max_vector_size() { return 16384; };

/**
* \brief Constructor with initialisation for Convergence plotter window (Tools/Convergence Plotter)
*/
ConvergencePlotter::ConvergencePlotter(Worker *appWorker, std::shared_ptr<Formulas> formulas)
        : GLWindow(), views{} {

    int wD = 625;
    int hD = 375;

    SetTitle("Convergence plotter");
    SetIconfiable(true);
    nbView = 0;
    worker = appWorker;
    appFormulas = formulas;

    lastUpdate = 0.0f;

    chart = new GLChart(0);
    chart->SetBorder(BORDER_BEVEL_IN);
    chart->GetY1Axis()->SetGridVisible(true);
    chart->GetXAxis()->SetGridVisible(true);
    chart->GetY1Axis()->SetAutoScale(true);
    chart->GetY2Axis()->SetAutoScale(true);
    chart->GetY1Axis()->SetAnnotation(VALUE_ANNO);
    chart->GetXAxis()->SetAnnotation(VALUE_ANNO);
    chart->GetXAxis()->SetName("Number of desorptions");

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

// copy constructor
ConvergencePlotter::ConvergencePlotter(const ConvergencePlotter& copy) : ConvergencePlotter(copy.worker, copy.appFormulas) {
    //nbView = copy.nbView;
    worker = copy.worker;
    appFormulas = copy.appFormulas;

    lastUpdate = 0.0f;

    profCombo->SetSelectedIndex(copy.profCombo->GetSelectedIndex());
    logYToggle->SetState(copy.logYToggle->GetState());
    colorToggle->SetState(copy.colorToggle->GetState());

    if (!colorToggle->GetState())
        chart->SetColorSchemeDefault();
    else
        chart->SetColorSchemeColorblind();

    SetViews(copy.GetViews());
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

    int lineHeightDiff = 45;
    formulaText->SetBounds(7, h - lineHeightDiff, 350, 19);
    formulaBtn->SetBounds(360, h - lineHeightDiff, 120, 19);;
    dismissButton->SetBounds(w - 100, h - lineHeightDiff, 90, 19);

    lineHeightDiff += 25;
    colorToggle->SetBounds(7, h - lineHeightDiff, 105, 19);
    fixedLineWidthText->SetBounds(112, h - lineHeightDiff, 93, 19);
    fixedLineWidthField->SetBounds(206, h - lineHeightDiff, 30, 19);
    fixedLineWidthButton->SetBounds(240, h - lineHeightDiff, 100, 19);
    pruneEveryNButton->SetBounds(w - 215, h - lineHeightDiff, 100, 19);
    pruneFirstNButton->SetBounds(w - 110, h - lineHeightDiff, 100, 19);

    lineHeightDiff += 25;
    profCombo->SetBounds(7, h - lineHeightDiff, 160, 19);
    logYToggle->SetBounds(180, h - lineHeightDiff, 40, 19);
    addButton->SetBounds(w - 270, h - lineHeightDiff, 80, 19);
    removeButton->SetBounds(w - 180, h - lineHeightDiff, 80, 19);
    removeAllButton->SetBounds(w - 90, h - lineHeightDiff, 80, 19);

    GLWindow::SetBounds(x, y, w, h);

}

/**
* \brief Refreshes all window components (combo, chart, values)
*/
void ConvergencePlotter::Refresh() {

    //Rebuild selection combo box
    size_t nbFormulas = appFormulas->formulas.size(); // minimum 1 for custom input
    profCombo->Clear();
    if (nbFormulas) {
        profCombo->SetSize(nbFormulas);
        for (size_t i = 0; i < nbFormulas; i++) {
            profCombo->SetValueAt(i, fmt::format("[{}] {}",i + 1, appFormulas->formulas[i].GetExpression()), (int) i);
        }
        profCombo->SetEditable(true);
    } else {
        profCombo->SetSize(1);
        profCombo->SetValueAt(0, "No formula found", (int) -1);
        profCombo->SetEditable(false);
    }
    profCombo->SetSelectedIndex(0);

    //Remove profiles that aren't present anymore
    for (int v = nbView - 1; v >= 0; v--) { //int because it can be -1, nbView is also int
        if (appFormulas->formulas.empty()) {
            chart->GetY1Axis()->RemoveDataView(views[v]);
            SAFE_DELETE(views[v]);
            for (size_t j = v; j < nbView - 1; j++) views[j] = views[j + 1];
            nbView--;
            continue;
        }
        int formId = 0;
        while (views[v]->userData1 !=
               (int) std::hash<std::string>{}(appFormulas->formulas[formId].GetExpression())) {
            ++formId;
            if (formId >= appFormulas->formulas.size()) {
                chart->GetY1Axis()->RemoveDataView(views[v]);
                SAFE_DELETE(views[v]);
                for (size_t j = v; j < nbView - 1; j++) views[j] = views[j + 1];
                nbView--;
                break;
            }
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
    Refresh();
    SetVisible(true);

}

/**
* \brief Refreshes the view if needed
* \param appTime current time of the application
*/
void ConvergencePlotter::Update(float appTime) {
    if (appFormulas->formulas.empty() || !appFormulas->recordConvergence || !nbView) {
        return;
    }

    refreshViews();
    lastUpdate = appTime;

    return;
}

/**
* \brief Creates a plot from the expression given in the textbox of the form f(x)=EXPRESSION (e.g. 2*x+50)
*/
void ConvergencePlotter::plot() {

    GLFormula formula;
    formula.SetExpression(formulaText->GetText().c_str());
    if (!formula.Parse()) {
        GLMessageBox::Display(formula.GetParseErrorMsg().c_str(), "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }

    int nbVar = formula.GetNbVariable();
    if (nbVar == 0) {
        GLMessageBox::Display("Variable 'x' not found", "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }
    if (nbVar > 1) {
        GLMessageBox::Display("Too much variables or unknown constant", "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }
    auto xVariable = formula.GetVariableAt(0);
    if (!iequals(xVariable->varName, "x")) {
        GLMessageBox::Display("Variable 'x' not found", "Error", GLDLG_OK, GLDLG_ICONERROR);
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
    } else {

        if (nbView < 50) {
            v = new GLDataView();
            v->SetName(formulaText->GetText().c_str());
            v->userData1 = -1;
            chart->GetY1Axis()->AddDataView(v);
            views[nbView] = v;
            nbView++;
        } else {
            return;
        }
    }

    // Plot
    for (i = 0; i < 1000; i++) {
        double x = (double)i;
        xVariable->value = x;
        try {
            v->Add(x, formula.Evaluate());
        }
        catch (...) {
            continue; //Eval. error, but maybe for other x it is possible to evaluate (ex. div by zero)
        }
    }
    v->CommitChange();

}

/**
* \brief Refreshes view by updating the data for the plot
*/
void ConvergencePlotter::refreshViews() {

    // Lock during update

    for (int i = 0; i < nbView; i++) {

        GLDataView *v = views[i];
        if (appFormulas->formulas.empty()) return;
        int formId = 0;
        while (v->userData1 != (int) std::hash<std::string>{}(appFormulas->formulas[formId].GetExpression())) {
            ++formId;
            if (formId >= appFormulas->formulas.size())
                return;
            if (formId >= appFormulas->convergenceData.size())
                return;
        }

        v->Reset();
        if (worker->globalStatCache.globalHits.nbDesorbed > 0) {
            const auto& conv_vec = appFormulas->convergenceData[formId];
            for (int j = std::max(0,(int)conv_vec.size()-1000); j < conv_vec.size(); j++) // limit data points to last 1000
                v->Add(conv_vec[j].nbDes, conv_vec[j].value, false);
        }
        v->CommitChange();

    }
}

/**
* \brief Adds view/plot for a specific facet
* \param formulaHash specific facet ID
* \return 0 if okay, 1 if already plotted
*/
int ConvergencePlotter::addView(int formulaHash) {

    if (appFormulas->formulas.empty()) return 0;

    // Check that view is not already added
    bool found = false;
    int i = 0;
    while (i < nbView && !found) {
        found = (views[i]->userData1 == formulaHash);
        if (!found) i++;
    }
    if (found) {
        return 1;
    }
    if (nbView < MAX_VIEWS) {
        found = false;
        GLFormula* formula;
        for (i = 0; !found && i < appFormulas->formulas.size();i++) {
            formula = &(appFormulas->formulas[i]);
            int str_hash = std::hash<std::string>{}(formula->GetExpression());
            if (str_hash == formulaHash) {
                found = true;
            }
        }
        if (!found) return 0;
        //if (i > appFormulas->convergenceData.size()) return 0; //No data for this formula
        GLDataView *v = new GLDataView();
        v->SetName(formula->GetExpression().c_str());
        //Look for first available color
        GLColor col = chart->GetFirstAvailableColor();
        int lineStyle = chart->GetFirstAvailableLinestyle(col);
        v->SetColor(col);
        v->SetMarkerColor(col);
        v->SetStyle(lineStyle);
        v->SetLineWidth(2);
        v->userData1 = formulaHash;

        chart->GetY1Axis()->AddDataView(v);
        views[nbView] = v;
        nbView++;
    }

    return 0;
}

/**
* \brief Removes view/plot for a specific facet
* \param formulaHash specific facet ID
* \return 0 if okay, 1 if not plotted
*/
int ConvergencePlotter::remView(int formulaHash) {

    bool found = false;
    int i = 0;
    while (i < nbView && !found) {
        found = (views[i]->userData1 == formulaHash);
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
            } else if (src == pruneEveryNButton) {
                for (int formulaId = 0; formulaId < appFormulas->formulas.size(); ++formulaId) {
                    appFormulas->removeEveryNth(4, formulaId, 0);
                }
                refreshViews();
            } else if (src == pruneFirstNButton) {
                for (int formulaId = 0; formulaId < appFormulas->formulas.size(); ++formulaId) {
                    appFormulas->removeFirstN(100, formulaId);
                }
                refreshViews();
            } else if (src == addButton) {
                int idx = profCombo->GetSelectedIndex();
                if (idx >= 0 && !appFormulas->formulas.empty()) { //Something selected (not -1)
                    if(appFormulas->formulas[profCombo->GetUserValueAt(idx)].hasEvalError){
                        GLMessageBox::Display(fmt::format("Formula can't be evaluated:\n{}", appFormulas->formulas[profCombo->GetUserValueAt(idx)].GetEvalErrorMsg()).c_str(), "Error", GLDLG_OK, GLDLG_ICONERROR);
                        break;
                    }
                    int str_hash = std::hash<std::string>{}(
                            appFormulas->formulas[profCombo->GetUserValueAt(idx)].GetExpression());
                    if (addView(str_hash))
                        GLMessageBox::Display("Profile already plotted", "Info", GLDLG_OK, GLDLG_ICONINFO);
                    refreshViews();
                }
            } else if (src == removeButton) {

                int idx = profCombo->GetSelectedIndex();
                if (idx >= 0 && !appFormulas->formulas.empty()) { //Something selected (not -1)
                    int str_hash = std::hash<std::string>{}(
                            appFormulas->formulas[profCombo->GetUserValueAt(idx)].GetExpression());
                    if (remView(str_hash)) {
                        GLMessageBox::Display("Profile not plotted", "Error", GLDLG_OK, GLDLG_ICONERROR);
                    }
                    refreshViews();
                }
            } else if (src == removeAllButton) {
                Reset();
            } else if (src == formulaBtn) {
                plot();
            } else if (src == fixedLineWidthButton) {
                int linW;
                fixedLineWidthField->GetNumberInt(&linW);
                for (int viewId = 0; viewId < nbView; viewId++) {
                    GLDataView *v = views[viewId];
                    v->SetLineWidth(linW);
                }
            }
            break;
        case MSG_COMBO:
            if (src == profCombo) {
                int profMode = profCombo->GetSelectedIndex();
            }
            break;
        case MSG_TOGGLE:
            if (src == logYToggle) {
                chart->GetY1Axis()->SetScale(logYToggle->GetState());
            } else if (src == colorToggle) {
                if (!colorToggle->GetState())
                    chart->SetColorSchemeDefault();
                else
                    chart->SetColorSchemeColorblind();

                const auto &colors = chart->GetColorScheme();
                for (int viewId = 0; viewId < nbView; viewId++) {
                    GLDataView *v = views[viewId];
                    auto col = colors[viewId % colors.size()];
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
        addView(view);
    //Refresh(); //Commented out: at this point, simulation results are not yet loaded
}

/**
* \brief Create and return a vector of view IDs
* \return vector containing the IDs of the views
*/
std::vector<int> ConvergencePlotter::GetViews() const {
    std::vector<int> v;
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
void ConvergencePlotter::SetLogScaled(bool logScale) {
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