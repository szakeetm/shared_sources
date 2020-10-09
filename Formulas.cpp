//
// Created by pbahr on 09/10/2020.
//

#include "Formulas.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow *mApp;
#elif defined(SYNRAD)
#include "../../src/SynRad.h"
extern SynRad *mApp;
#endif

bool Formulas::InitializeFormulas(){
    bool allOk = true;
    for (size_t i = 0; i < formulas_n.size(); i++) {

        // Evaluate variables
        int nbVar = formulas_n.at(i)->GetNbVariable();
        bool ok = true;
        for (int j = 0; j < nbVar && ok; j++) {
            VLIST *v = formulas_n.at(i)->GetVariableAt(j);
            ok = mApp->EvaluateVariable(v);
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