//
// Created by pbahr on 09/10/2020.
//

#include "Formulas.h"

bool Formulas::InitializeFormulas(){
    bool allOk = true;
    for (size_t i = 0; i < formulas_n.size(); i++) {

        // Evaluate variables
        int nbVar = formulas_n.at(i)->GetNbVariable();
        bool ok = true;
        for (int j = 0; j < nbVar && ok; j++) {
            VLIST *v = formulas_n.at(i)->GetVariableAt(j);
            ok = evaluator->EvaluateVariable(v);
        }

        if (ok) {
            formulas_n.at(i)->hasVariableEvalError = false;
        }
        else{
            allOk = false;
        }
    }
    return allOk;
};