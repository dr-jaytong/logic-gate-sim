#ifndef PARSE_VERILOG_FILE
#define PARSE_VERILOG_FILE

#include <vector>
#include <string>

#include "FileHandler.hpp"

class ParseVerilogFile
{
public:    
    void ParseFile(std::string const &sFileName);
    std::vector<std::string> ExtractPortNames(FileHandler &VerilogFile);
    std::vector<std::string> ExtractPortNames(FileHandler &VerilogFile, std::string const &sPortsFromString);
    void ExtractGate(FileHandler &VerilogFile, std::string const &sInput);
    bool IsGate(std::string const &sLine);
    std::string ParseNextLine(FileHandler &VerilogFile);
};

#endif
