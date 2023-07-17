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

struct ConvergenceData {
    std::vector<std::pair<size_t,double>> valueHistory; //series of nbDes,value pairs 
    // ASCBR values
    double upper_bound=0.0;
    double lower_bound=0.0;
    size_t chain_length=0.0;
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
    void EvaluateFormulaVariables(size_t formulaIndex, const std::vector <std::pair<std::string, std::optional<double>>>& previousFormulaValues);
    void EvaluateFormulas(size_t nbDesorbed);
    bool FetchNewConvValue();
    //double GetConvRate(int formulaId);
    //void RestartASCBR(int formulaId);
    //bool CheckASCBR(int formulaId);

    void removeEveryNth(size_t everyN, int formulaId, size_t skipLastN);
    void removeFirstN(size_t n, int formulaId);

    std::vector<GLFormula> formulas;
    std::vector<std::pair<size_t,double>> previousFormulaValues; //nbDesorbed,value
    std::vector<ConvergenceData> convergenceValues; // One per formula
    //std::vector<size_t> freq_accum;
    bool convergenceDataChanged=true;
    bool recordConvergence=true;

    //bool useAbsEps=true;
    //int epsilon=5;
    //size_t cb_length=51;
    std::shared_ptr<FormulaEvaluator> evaluator=nullptr;
};
