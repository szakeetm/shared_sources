#pragma once
#include "GLApp/GLParser.h"


//! Class interface for the application specific evaluation of formula variables, e.g. as used by the @see FormulaEditor or @see ConvergencePlotter
class FormulaEvaluator {
public:
    virtual ~FormulaEvaluator() = default;
    virtual bool EvaluateVariable(VLIST *v) { return false; } //! Abstract method used for application specific evaluation of a given variable

protected:
    int GetVariable(const char *name, const char *prefix);
};