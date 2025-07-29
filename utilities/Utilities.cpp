#include "Utilities.hpp"
#include <chrono>
#include <sstream>
#include <algorithm>

namespace Utility {
namespace String {

std::string GetFirstWord(std::string const &sLine)
{
    size_t const iStartPosition(sLine.find_first_not_of(" \t\n\r\f\v"));             // Find first non-white space character
    size_t const iEndPosition  (sLine.find_first_of(" \t\n\r\f\v", iStartPosition)); // Find first position of white space after first character

    if (iEndPosition == std::string::npos)
        return sLine.substr(iStartPosition, sLine.length());

    return RemoveWhiteSpace(sLine.substr(iStartPosition, iEndPosition - iStartPosition));
}

std::string RemoveWhiteSpace(std::string const &sLine)
{
    return Strip(sLine, ' ');
}

std::string Strip(std::string const &sLine, std::vector<char> const &vPatterns)
{
    std::string sCleanLine(sLine);
    for (auto const &sPattern : vPatterns) 
        sCleanLine = Strip(sCleanLine, sPattern);

    return sCleanLine;
}

std::string Strip(std::string const &sLine, char const &cDelimeter)
{
    std::string sCleanLine(sLine);
    sCleanLine.erase(std::remove(sCleanLine.begin(), sCleanLine.end(), cDelimeter), sCleanLine.end());
    return sCleanLine; 
}

std::string Strip(std::string const &sLine, std::string const &sWordToRemove)
{
    std::string sCleanLine(sLine);
    size_t const uiPos(sCleanLine.find(sWordToRemove));

    if (uiPos != std::string::npos) 
        sCleanLine.erase(uiPos, sWordToRemove.length());

    return sCleanLine;
}

std::vector<std::string> Tokenize(std::string const &sLine, char const &cDelimeter)
{
    std::vector<std::string> vTokens;
    std::stringstream ssLine(sLine);
    std::string sToken("");
    
    while (std::getline(ssLine, sToken, cDelimeter)) {
        vTokens.emplace_back(sToken);
    }

    return vTokens;
}

std::string ToLowerCase(std::string const &sInput)
{
    std::string sConverted(sInput);
    std::transform(sConverted.begin(), sConverted.end(), sConverted.begin(), ::tolower); 
    return sConverted;
}

}; // namespace string

std::string PrintElapsedTime(std::chrono::steady_clock::time_point const &tpEnd,
                             std::chrono::steady_clock::time_point const &tpStart)
{
    std::chrono::steady_clock::duration tDuration(tpEnd - tpStart);
    long long llTotalTimeInSeconds(std::chrono::duration_cast<std::chrono::seconds>(tDuration).count());
    long long const llMinutes(llTotalTimeInSeconds % 3600 / 60);
    long long const llSeconds(llTotalTimeInSeconds % 60);
    double const llMilliSeconds(llTotalTimeInSeconds % 1000);
    return std::to_string(llMinutes) + ":" + std::to_string(llSeconds) + "." + std::to_string(llMilliSeconds);
}
    
};
