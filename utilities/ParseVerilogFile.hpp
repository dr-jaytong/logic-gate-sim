#ifndef PARSE_VERILOG_FILE
#define PARSE_VERILOG_FILE

#include <vector>
#include <string>

#include "FileHandler.hpp"

class ParseVerilogFile
{
private:
    FileHandler m_VerilogFile;

    std::string ParseNextLine();
    bool IsGate(std::string const &sLine);
    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);
public:

    explicit ParseVerilogFile(std::string const sFileName) : m_VerilogFile(sFileName) { ParseFile(); }
   ~ParseVerilogFile(){}

    void ParseFile();
    void ExtractGateData();
    void ExtractVerilogData();
    void ExtractModulePorts();
};

#endif
