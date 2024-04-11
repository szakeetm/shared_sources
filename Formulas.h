

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
