

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