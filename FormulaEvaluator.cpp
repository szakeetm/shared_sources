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
#include <charconv>

/**
* \brief Returns the index for a facet of a variable
* \param name char string containing the full variable including index
* \param prefix char string containing the prefix for the variable that name should be checked against for validity
* \return integer describing index of the variable
* \example: GetFacetIndex("D12","D") returns 12
*/
int FormulaEvaluator::GetFacetIndex(const std::string &varName, const std::string& prefix) {

    if (prefix.length() >= varName.length()) return -1;
    
    if (iBeginsWith(varName, lowercase(prefix))) {
        std::string remainder = varName.substr(prefix.length());
        int facetId;
        auto result = std::from_chars(remainder.data(), remainder.data() + remainder.size(), facetId);
        // Check if the conversion was successful
        if (result.ec == std::errc()) {
            // Conversion successful
            return facetId;
        }
        else {
            // Conversion failed: prefix doesn't match
            return -1;
        }
    } else {
        return -1; //Does not match prefix
    }
}