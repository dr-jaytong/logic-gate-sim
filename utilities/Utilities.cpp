#include "Utilities.hpp"
#include <sstream>
#include <algorithm>

namespace Utilities {

std::string GetFirstWordFromLine(std::string const &sLine)
{
    size_t const iStartPosition(sLine.find_first_not_of(" \t\n\r\f\v"));             // Find first non-white space character
    size_t const iEndPosition  (sLine.find_first_of(" \t\n\r\f\v", iStartPosition)); // Find first position of white space after first character

    if (iEndPosition == std::string::npos)
        return sLine.substr(iStartPosition, sLine.length());

    return RemoveWhiteSpace(sLine.substr(iStartPosition, iEndPosition - iStartPosition));
}

std::string RemoveWhiteSpace(std::string const &sLine)
{
    return StripString(sLine, ' ');
}

std::string StripString(std::string const &sLine, std::vector<char> const &vPatterns)
{
    std::string sCleanLine(sLine);
    for (auto const &sPattern : vPatterns) {
        sCleanLine = StripString(sCleanLine, sPattern);

    }


    return sCleanLine;
}

std::string StripString(std::string const &sLine, char const &cDelimeter)
{
    std::string sCleanLine(sLine);
    sCleanLine.erase(std::remove(sCleanLine.begin(), sCleanLine.end(), cDelimeter), sCleanLine.end());
    return sCleanLine; 
}

std::string StripString(std::string const &sLine, std::string const &sWordToRemove)
{
    std::string sCleanLine(sLine);
    size_t const uiPos(sCleanLine.find(sWordToRemove));

    if (uiPos != std::string::npos) {
        sCleanLine.erase(uiPos, sWordToRemove.length());
    }

    return sCleanLine;
}

std::vector<std::string> TokenizeString(std::string const &sLine, char const &cDelimeter)
{
    std::vector<std::string> vTokens;
    std::stringstream ssLine(sLine);
    std::string sToken("");
    
    while (std::getline(ssLine, sToken, cDelimeter)) {
        vTokens.emplace_back(sToken);
    }

    return vTokens;
}

};
