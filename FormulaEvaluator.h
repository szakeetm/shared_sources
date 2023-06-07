#pragma once
#include "GLApp/GLParser.h"


//! Class interface for the application specific evaluation of formula variables, e.g. as used by the @see FormulaEditor or @see ConvergencePlotter
class FormulaEvaluator {
public:
    virtual bool EvaluateVariable(std::list<Variable>::iterator v) = 0; //! Abstract method used for application specific evaluation of a given variable

protected:
    int GetFacetIndex(const std::string& varName, const std::string& prefix);
};