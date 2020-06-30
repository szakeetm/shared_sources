//
// Created by pbahr on 18/06/2020.
//

#ifndef MOLFLOW_PROJ_STRINGHELPER_H
#define MOLFLOW_PROJ_STRINGHELPER_H

#include <string>
#include <vector>

void splitFacetList(std::vector<size_t>& outputFacetIds, std::string inputString, size_t nbFacets);
std::string AbbreviateString(const std::string& input, size_t maxLength);


#endif //MOLFLOW_PROJ_STRINGHELPER_H
