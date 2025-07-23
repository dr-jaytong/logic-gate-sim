#ifndef PARSE_VERILOG_FILE
#define PARSE_VERILOG_FILE

#include <vector>
#include <string>

#include "Verilog.hpp"
#include "FileHandler.hpp"

namespace VerilogUtility
{
    void ParseFile(Verilog &VerilogModule, FileHandler &VerilogFile);
    std::string ParseNextVerilogLine(FileHandler &VerilogFile);
    bool IsGate(std::string const &sLine);

    void ExtractVerilogData(Verilog &VerilogModule, FileHandler &VerilogFile);
    Verilog::Gate ExtractGateData(std::string const &sGateDataFromString);
    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);

    void ExtractConnections(std::string const &sNamesFromString, Verilog &verilogmodule);
};

#endif
