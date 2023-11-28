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

#pragma once

#include <memory>
#include <vector>
#include "Buffer_shared.h"
#include "FormulaEvaluator.h"
class Worker;

//App storage of GLFormula with helper methods and convergence stuff
struct Formulas {

    Formulas(std::shared_ptr<FormulaEvaluator> eval) {
        evaluator=eval;
    };

    void AddFormula(const std::string& name, const std::string& expression);
    void ClearFormulas();
    void ResetConvergenceData();
    void UpdateVectorSize();
    void EvaluateFormulaVariables(size_t formulaIndex, const std::vector <std::pair<std::string, std::optional<double>>>& aboveFormulaValues);
    void EvaluateFormulas(size_t nbDesorbed);
    bool RecordNewConvergenceDataPoint();
    std::string GetFormulaValue(int index);
    std::string ExportCurrentFormulas();
    std::string ExportFormulasAtAllMoments(Worker* worker);

    void removeEveryNth(size_t everyN, int formulaId, size_t skipLastN);
    void removeFirstN(size_t n, int formulaId);

    std::vector<GLFormula> formulas;
    std::vector<FormulaHistoryDatapoint> formulaValueCache; //One per formula. Keeps Evaluate() results (From EvaluateFormulas()) in memory so convergenc values can be recorded
    std::vector<std::vector<FormulaHistoryDatapoint>> convergenceData; // One per formula
    bool convergenceDataChanged=true;
    bool recordConvergence=true;
    std::shared_ptr<FormulaEvaluator> evaluator=nullptr;
};
