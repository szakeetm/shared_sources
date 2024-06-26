

#ifndef MOLFLOW_PROJ_STRINGHELPER_H
#define MOLFLOW_PROJ_STRINGHELPER_H

#include <string>
#include <vector>

template <class T>
T stringToNumber(const std::string& s, bool returnDefValOnErr=false);

extern template int stringToNumber<int>(const std::string& s, bool returnDefValOnErr);
extern template size_t stringToNumber<size_t>(const std::string& s, bool returnDefValOnErr);
extern template double stringToNumber<double>(const std::string& s, bool returnDefValOnErr);

void splitList(std::vector<size_t>& outputIds, std::string inputString, size_t upperLimit);
void splitFacetList(std::vector<size_t>& outputFacetIds, std::string inputString, size_t nbFacets);
std::string AbbreviateString(const std::string& input, size_t maxLength);
std::vector<std::string> SplitString(const std::string & input);
std::vector<std::string> SplitString(const std::string & input, char delimiter);

bool beginsWith(const std::string& fullString, const std::string& beginning);
bool endsWith(const std::string& fullString, const std::string& ending);
bool iBeginsWith(const std::string& fullString, const std::string& beginning);
bool iEndsWith(const std::string& fullString, const std::string& ending);

std::string findAndReplace(std::string text, std::string find, std::string replace);
std::string space2underscore(std::string text);
std::string molflowToAscii(std::string text);
std::string molflowToUnicode(std::string text);
bool iequals(const std::string& a, const std::string& b);
std::string lowercase(const std::string& s);
std::string uppercase(const std::string& s);
bool iContains(const std::vector<std::string>& vec, const std::string& value);
std::string FlattenLines(const std::vector<std::string>& lines);
size_t countLines(const std::string& str, bool countEmpty=true);
size_t countLines(const std::stringstream& ss, bool countEmpty=true);

namespace Util {
    std::string getTimepointString();

    template<typename T>
    bool getNumber(T* num, std::string str)
    {
        try {
            *num = static_cast<T>(std::stod(str));
            return true;
        }
        catch (std::exception e)
        {
            return false;
        }
    }
}
#endif //MOLFLOW_PROJ_STRINGHELPER_H
