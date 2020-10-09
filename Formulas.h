//
// Created by pbahr on 09/10/2020.
//

#ifndef MOLFLOW_PROJ_FORMULAS_H
#define MOLFLOW_PROJ_FORMULAS_H

#include <memory>
#include <vector>
#include "GLApp/GLParser.h"

struct Formulas {
    std::vector<GLParser*> formulas_n;
    std::vector<std::vector<std::pair<size_t,double>>> convergenceValues; // One vector of nbDesorption,formulaValue pairs for each formula
    bool formulasChanged;
};
#endif //MOLFLOW_PROJ_FORMULAS_H
