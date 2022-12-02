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
std::string lowercase(const std::string& s);
std::string uppercase(const std::string& s);
bool iContains(const std::vector<std::string>& vec, const std::string& value);

namespace Util {
    std::string getTimepointString();
}
#endif //MOLFLOW_PROJ_STRINGHELPER_H
