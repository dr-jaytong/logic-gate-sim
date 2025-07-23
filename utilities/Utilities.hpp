#include <string>
#include <vector>
#include <chrono>

namespace Utility
{
namespace String
{
    std::string GetFirstWord(std::string const &sLine);
    std::string Strip(std::string const &sLine, std::vector<char> const &vPatterns);
    std::string Strip(std::string const &sLine, char const &cDelimeter);
    std::string Strip(std::string const &sLine, std::string const &sWord);
    std::string RemoveWhiteSpace(std::string const &sLine);
    std::string ToLowerCase(std::string const &sInput);

    std::vector<std::string> Tokenize(std::string const &sLine, char const &cDelimeter);
};

    std::string PrintElapsedTime(std::chrono::steady_clock::time_point const &tpEnd,
                                 std::chrono::steady_clock::time_point const &tpStart);
};
