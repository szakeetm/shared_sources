//
// Created by pbahr on 18/06/2020.
//

#include <stdexcept>
#include <sstream>
#include <numeric>
#include <GLApp/GLTypes.h>
#include "GLApp/MathTools.h"
#include "StringHelper.h"

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
            catch (std::invalid_argument arg) {
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
            catch (std::invalid_argument arg) {
                std::ostringstream tmp;
                tmp << "Invalid facet number " << tokens[0] << "\n" << arg.what();
                throw std::invalid_argument(tmp.str());
            }
            catch (Error& err) {
                throw std::runtime_error(err.what());
            }
        }
    }

    return;
}

std::string AbbreviateString(const std::string& input, size_t maxLength)
{
    //abbreviates string by replacing middle part with ...
    //intended to abbreviate long paths
    //maxLength defines returned string length if input is longer

    if (maxLength>=input.length()) return std::string(input); //return a copy, not a reference to the input
    if (maxLength <= 5) return "...";
    size_t fromBeginning = (maxLength - 3) / 2;
    size_t fromEnd = (maxLength - 2) / 2;
    std::string result = input.substr(0, fromBeginning) + "..." + input.substr(input.length() - fromEnd);
    return result;
}
