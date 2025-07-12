#include <string>
#include <vector>


namespace Utility
{
    std::string GetFirstWordFromLine(std::string const &sLine);
    std::string StripString(std::string const &sLine, std::vector<char> const &vPatterns);
    std::string StripString(std::string const &sLine, char const &cDelimeter);
    std::string StripString(std::string const &sLine, std::string const &sWord);
    std::string RemoveWhiteSpace(std::string const &sLine);
    std::vector<std::string> TokenizeString(std::string const &sLine, char const &cDelimeter);
};
