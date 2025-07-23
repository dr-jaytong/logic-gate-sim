#ifndef PARSE_VERILOG_FILE
#define PARSE_VERILOG_FILE

#include <vector>
#include <string>
#include <unordered_map>

#include "Verilog.hpp"
#include "FileHandler.hpp"

namespace VerilogUtility
{
    void ParseFile(Verilog &VerilogModule, FileHandler &VerilogFile);
    std::string ParseNextVerilogLine(FileHandler &VerilogFile);
    bool IsGate(std::string const &sLine);
    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);

    Verilog::Gate ExtractLogicData(std::string const &sGateDataFromString);
    std::unordered_map<std::string, Verilog::Connection> ExtractConnections(std::string const &sNamesFromString);

};

#endif
