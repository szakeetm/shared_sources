//
// Created by pbahr on 12/10/2020.
//

#include "FormulaEvaluator.h"
#include <Helper/StringHelper.h>

int FormulaEvaluator::GetVariable(const char *name, const char *prefix) {
    char tmp[256];
    int idx;
    int lgthP = (int) strlen(prefix);
    int lgthN = (int) strlen(name);

    if (lgthP >= lgthN) {
        return -1;
    } else {
        strcpy(tmp, name);
        tmp[lgthP] = 0;

        if (iequals(tmp, prefix)) {
            strcpy(tmp, name + lgthP);
            int conv = sscanf(tmp, "%d", &idx);
            if (conv) {
                return idx;
            } else {
                return -1;
            }
        }
    }
    return -1;
}