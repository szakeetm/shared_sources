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

#include "StringHelper.h"
#include "MathTools.h"

#include <stdexcept>
#include <sstream>
#include <numeric>
#include <iterator>

#include <GLApp/GLTypes.h>
#include <chrono>
#ifdef _WIN32
#include <ctime> // for time_t
#endif
// String to Number parser, on fail returns a default value (returnDefValOnErr==true) or throws an error
template <class T>
T stringToNumber(std::string const& s, bool returnDefValOnErr) {
    std::istringstream i(s);
    T x;
    if (!(i >> x)) {
        if(returnDefValOnErr)
            return T();
        else
            throw std::logic_error("stringToNumber(\"" + s + "\")");
    }
    return x;
}

template int stringToNumber<int>(std::string const& s, bool returnDefValOnErr);
template size_t stringToNumber<size_t>(std::string const& s, bool returnDefValOnErr);
template double stringToNumber<double>(std::string const& s, bool returnDefValOnErr);

void splitList(std::vector<size_t>& outputIds, std::string inputString, size_t upperLimit) {
    auto ranges = SplitString(inputString, ',');
    if (ranges.size() == 0) {
        throw std::logic_error("Can't parse input");
    }
    for (const auto& range : ranges) {
        auto tokens = SplitString(range, '-');
        if (!Contains({ 1,2 },tokens.size())) {
            std::ostringstream tmp;
            tmp << "Can't parse \"" << range << "\". Input should be a number or a range.";
            throw std::invalid_argument(tmp.str());
        }
        else if (tokens.size() == 1) {
            //One facet
            try {
                int splitId = std::stoi(tokens[0]);
                std::ostringstream tmp;
                tmp << "Wrong id " << tokens[0];
                if (splitId <= 0 || splitId > upperLimit) throw std::invalid_argument(tmp.str());
                outputIds.push_back(splitId - 1);
            }
            catch (std::invalid_argument arg) {
                std::ostringstream tmp;
                tmp << "Invalid number " << tokens[0] <<"\n" << arg.what();
                throw std::invalid_argument(tmp.str());
            }
        }
        else if (tokens.size() == 2) {
            //Range
            try {
                std::ostringstream tmp;
                int id1 = std::stoi(tokens[0]);
                if (id1 <= 0 || id1 > upperLimit) {
                    tmp << "Wrong id " << tokens[0];
                    throw std::invalid_argument(tmp.str());
                }
                int id2 = std::stoi(tokens[1]);
                if (id2 <= 0 || id2 > upperLimit) {
                    tmp << "Wrong id " << tokens[1];
                    throw std::invalid_argument(tmp.str());
                }
                if (id2 <= id1) {
                    tmp << "Invalid range " << id1 << "-" << id2;
                    throw std::invalid_argument(tmp.str());
                }
                size_t oldSize = outputIds.size();
                outputIds.resize(oldSize + id2 - id1 + 1);
                std::iota(outputIds.begin() + oldSize, outputIds.end(), id1 - 1);
            }
            catch (std::invalid_argument arg) {
                std::ostringstream tmp;
                tmp << "Invalid input number " << tokens[0] << "\n" << arg.what();
                throw std::invalid_argument(tmp.str());
            }
            catch (const std::exception &e) {
                throw std::runtime_error(e.what());
            }
        }
    }

    return;
}

void splitFacetList(std::vector<size_t>& outputFacetIds, std::string inputString, size_t nbFacets) {
    auto ranges = SplitString(inputString, ',');
    if (ranges.size() == 0) {
        throw std::logic_error("Can't parse input");
    }
    for (const auto& range : ranges) {
        auto tokens = SplitString(range, '-');
        if (!Contains({ 1,2 },tokens.size())) {
            std::ostringstream tmp;
            tmp << "Can't parse \"" << range << "\". Should be a facet number or a range.";
            throw std::invalid_argument(tmp.str());
        }
        else if (tokens.size() == 1) {
            //One facet
            try {
                int facetId = std::stoi(tokens[0]);
                std::ostringstream tmp;
                tmp << "Wrong facet id " << tokens[0];
                if (facetId <= 0 || facetId > nbFacets) throw std::invalid_argument(tmp.str());
                outputFacetIds.push_back(facetId - 1);
            }
            catch (std::invalid_argument& arg) {
                std::ostringstream tmp;
                tmp << "Invalid facet number " << tokens[0] <<"\n" << arg.what();
                throw std::invalid_argument(tmp.str());
            }
        }
        else if (tokens.size() == 2) {
            //Range
            try {
                std::ostringstream tmp;
                int facetId1 = std::stoi(tokens[0]);
                if (facetId1 <= 0 || facetId1 > nbFacets) {
                    tmp << "Wrong facet id " << tokens[0];
                    throw std::invalid_argument(tmp.str());
                }
                int facetId2 = std::stoi(tokens[1]);
                if (facetId2 <= 0 || facetId2 > nbFacets) {
                    tmp << "Wrong facet id " << tokens[1];
                    throw std::invalid_argument(tmp.str());
                }
                if (facetId2 <= facetId1) {
                    tmp << "Invalid range " << facetId1 << "-" << facetId2;
                    throw std::invalid_argument(tmp.str());
                }
                size_t oldSize = outputFacetIds.size();
                outputFacetIds.resize(oldSize + facetId2 - facetId1 + 1);
                std::iota(outputFacetIds.begin() + oldSize, outputFacetIds.end(), facetId1 - 1);
            }
            catch (std::invalid_argument& arg) {
                std::ostringstream tmp;
                tmp << "Invalid facet number " << tokens[0] << "\n" << arg.what();
                throw std::invalid_argument(tmp.str());
            }
            catch (const std::exception &e) {
                throw std::runtime_error(e.what());
            }
        }
    }

    return;
}

/// <summary>
/// Abbreviates string by replacing middle part with "..."
/// </summary>
/// <param name="input">The string to abbreviate</param>
/// <param name="maxLength">The maximal returned string length</param>
/// <returns>Returns the original string if its length is smaller than maxLength, otherwise a maxLength long abbreviated string</returns>
std::string AbbreviateString(const std::string& input, size_t maxLength)
{
    if (maxLength>=input.length()) return std::string(input); //return a copy, not a reference to the input
    if (maxLength <= 5) return "...";
    size_t fromBeginning = (maxLength - 3) / 2;
    size_t fromEnd = (maxLength - 2) / 2;
    std::string result = input.substr(0, fromBeginning) + "..." + input.substr(input.length() - fromEnd);
    return result;
}

std::vector<std::string> SplitString(std::string const& input) {
    //Split string by whitespaces
    std::istringstream buffer(input);
    std::vector<std::string> ret;

    std::copy(std::istream_iterator<std::string>(buffer),
        std::istream_iterator<std::string>(),
        std::back_inserter(ret));
    return ret;
}

std::vector<std::string> SplitString(std::string const& input, const char& delimiter)
{
    std::vector<std::string> result;
    char* str = strdup(input.c_str());
    char* str_ptr = str; // keep to free memory
    do
    {
        char* begin = str;
        while (*str != delimiter && *str)
            str++;
        result.emplace_back(std::string(begin, str));
    } while (0 != *str++);
    free(str_ptr);
    return result;
}

bool endsWith(std::string const& fullString, std::string const& ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

bool beginsWith(std::string const& fullString, std::string const& beginning) {
    return (fullString.compare(0, beginning.length(), beginning) == 0);
}

std::string space2underscore(std::string text) {
    for (std::string::iterator it = text.begin(); it != text.end(); ++it) {
        if (*it == ' ') {
            *it = '_';
        }
    }
    return text;
}

std::string molflowToAscii(std::string text) {
    //Changes special characters in Molflow ASCII table to regular, exportable/copiable characters
    for (std::string::iterator it = text.begin(); it != text.end(); ++it) {
        if (*it == '\201') {
            *it = 'u';
        } else if (*it == '\202') {
            *it = 'v';
        } else if (*it == '\262') {
            *it = '2';
        }
        else if (*it == '\263') {
            *it = '3';
        }
    }
    return text;
}

bool iequals(std::string str1, std::string str2)
{
    //From https://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c
    return str1.size() == str2.size()
        && std::equal(str1.begin(), str1.end(), str2.begin(), [](auto a, auto b) {return std::tolower(a) == std::tolower(b);});
}

//
//  Lowercases string
//  Known to break with Unicode characters
//
std::string lowercase(const std::string& s)
{
    std::string s2 = s;
    std::transform(s2.begin(), s2.end(), s2.begin(), tolower);
    return s2;
}

//
// Uppercases string
// Known to break with Unicode characters
//
std::string uppercase(const std::string& s)
{
    std::string s2 = s;
    std::transform(s2.begin(), s2.end(), s2.begin(), toupper);
    return s2;
}

bool iContains(const std::vector<std::string>& vec, const std::string& value) { //Case insensitive "is value in vector". For example iContains({"Apple","Banana"},"BANANA"}) is true
    std::string lowercaseValue = lowercase(value);
    for (auto v = vec.begin(); v != vec.end(); v++) {
        if (lowercase(*v) == lowercaseValue) return true;
    }
    return false;
}

namespace Util {
    std::string getTimepointString(){
        auto time_point = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(time_point);
        char s[256];
        struct tm *p = localtime(&now_c);
        //YYYY.MM.DD_HH.MM.SS
        strftime(s, 256, "%F_%H.%M.%S", p);
        return s;
    }
}

