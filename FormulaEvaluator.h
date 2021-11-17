
#pragma once
#include "GLApp/GLParser.h"

class FormulaEvaluator {
public:
    virtual ~FormulaEvaluator() = default;
    virtual bool EvaluateVariable(VLIST *v) { return false; }

protected:
    int GetVariable(const char *name, const char *prefix);
};