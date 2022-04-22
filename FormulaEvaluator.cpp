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

#include "FormulaEvaluator.h"
#include <Helper/StringHelper.h>

/**
* \brief Returns the index for a facet of a variable
* \param name char string containing the full variable including index
* \param prefix char string containing the prefix for the variable that name should be checked against for validity
* \return integer describing index of the variable
*/
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