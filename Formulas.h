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
#include "GLApp/GLFormula.h"
#include "FormulaEvaluator.h"

struct FormulaHistoryDatapoint {
    FormulaHistoryDatapoint() = default; //So that a vector for this can be defined
    FormulaHistoryDatapoint(size_t _nbDes, double _value) : nbDes(_nbDes), value(_value) {};
    size_t nbDes=0;
    double value=0.0;
};

//App storage of GLFormula with helper methods and convergence stuff
struct Formulas {

    Formulas(std::shared_ptr<FormulaEvaluator> eval) {
        evaluator=eval;
        //freq_accum.resize(cb_length);
    };

    void AddFormula(const std::string& name, const std::string& expression);
    void ClearFormulas();

    void UpdateVectorSize();
    void EvaluateFormulaVariables(size_t formulaIndex, const std::vector <std::pair<std::string, std::optional<double>>>& aboveFormulaValues);
    void EvaluateFormulas(size_t nbDesorbed);
    bool RecordNewConvergenceDataPoint();
    //double GetConvRate(int formulaId);
    //void RestartASCBR(int formulaId);
    //bool CheckASCBR(int formulaId);

    void removeEveryNth(size_t everyN, int formulaId, size_t skipLastN);
    void removeFirstN(size_t n, int formulaId);

    std::vector<GLFormula> formulas;
    std::vector<FormulaHistoryDatapoint> formulaValueCache; //One per formula. Keeps Evaluate() results (From EvaluateFormulas()) in memory so convergenc values can be recorded
    std::vector<std::vector<FormulaHistoryDatapoint>> convergenceData; // One per formula
    //std::vector<size_t> freq_accum;
    bool convergenceDataChanged=true;
    bool recordConvergence=true;

    //bool useAbsEps=true;
    //int epsilon=5;
    //size_t cb_length=51;
    std::shared_ptr<FormulaEvaluator> evaluator=nullptr;
};
