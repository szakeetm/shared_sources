//
// Created by pbahr on 09/10/2020.
//

#ifndef MOLFLOW_PROJ_FORMULAS_H
#define MOLFLOW_PROJ_FORMULAS_H

#include <memory>
#include <vector>
#include "GLApp/GLParser.h"
#include "FormulaEvaluator.h"

struct ConvergenceData {
    ConvergenceData() : conv_total(0.0), n_samples(0), upper_bound(0.0), lower_bound(0.0), chain_length(0) {};
    std::vector<std::pair<size_t,double>> conv_vec;
    double conv_total; /* for now unused accumulator, where convegence values are summed up */
    size_t n_samples;

    // ASCBR values
    double upper_bound;
    double lower_bound;
    size_t chain_length;
};

struct Formulas {

    Formulas(FormulaEvaluator* eval) : formulasChanged(true), sampleConvValues(true), epsilon(5), cb_length(51), useAbsEps(true){
        evaluator=eval;
        freq_accum.resize(cb_length);
    };
    ~Formulas(){delete evaluator;};

    void AddFormula(const char *fName, const char *formula);
    void ClearFormulas();

    void UpdateVectorSize();
    bool InitializeFormulas();
    bool UpdateFormulaValues(size_t nbDesorbed);
    bool FetchNewConvValue();
    double GetConvRate(int formulaId);
    void RestartASCBR(int formulaId);
    bool CheckASCBR(int formulaId);
    double ApproxShapeParameter();
    double ApproxShapeParameter2();

    void pruneEveryN(size_t everyN, int formulaId, size_t skipLastN);
    void pruneFirstN(size_t n, int formulaId);

    std::vector<GLParser*> formulas_n;
    std::vector<std::pair<size_t,double>> lastFormulaValues;
    std::vector<ConvergenceData> convergenceValues; // One vector of nbDesorption,formulaValue pairs for each formula
    std::vector<size_t> freq_accum;
    bool formulasChanged;
    bool sampleConvValues;

    bool useAbsEps;
    int epsilon;
    size_t cb_length;
    FormulaEvaluator* evaluator;
};

#endif //MOLFLOW_PROJ_FORMULAS_H
