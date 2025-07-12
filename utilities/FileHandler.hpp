
#include <fstream>
#include <string>

class FileHandler
{
private:
    std::ifstream m_File;
    std::string m_sLastLineParsed;

public:

    static bool FileExists(std::string const &sFileName);

    explicit FileHandler(std::string const &sFileName);
    FileHandler() = delete;
   ~FileHandler();

    FileHandler           (FileHandler const &RHS) = delete;
    FileHandler &operator=(FileHandler const &RHS) = delete;

    std::string GetLastLineParsed() { return m_sLastLineParsed; }
    std::string GetNextLine();

    bool eof() const { return m_File.eof(); }
};
