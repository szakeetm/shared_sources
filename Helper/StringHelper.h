//
// Created by pbahr on 18/06/2020.
//

#ifndef MOLFLOW_PROJ_STRINGHELPER_H
#define MOLFLOW_PROJ_STRINGHELPER_H

#include <string>
#include <vector>

template <class T>
T stringToNumber(std::string const& s, bool returnDefValOnErr=false);

extern template int stringToNumber<int>(std::string const& s, bool returnDefValOnErr);
extern template size_t stringToNumber<size_t>(std::string const& s, bool returnDefValOnErr);
extern template double stringToNumber<double>(std::string const& s, bool returnDefValOnErr);

void splitList(std::vector<size_t>& outputIds, std::string inputString, size_t upperLimit);
void splitFacetList(std::vector<size_t>& outputFacetIds, std::string inputString, size_t nbFacets);
std::string AbbreviateString(const std::string& input, size_t maxLength);
std::vector<std::string> SplitString(std::string const& input);
std::vector<std::string> SplitString(std::string const& input, const char& delimiter);

bool endsWith(std::string const& fullString, std::string const& ending);
bool beginsWith(std::string const& fullString, std::string const& beginning);
std::string space2underscore(std::string text);
std::string molflowToAscii(std::string text);
bool iequals(std::string a, std::string b);

namespace Util {
    std::string getTimepointString();
}
#endif //MOLFLOW_PROJ_STRINGHELPER_H
