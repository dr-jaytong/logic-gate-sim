#ifndef PARSE_VERILOG_FILE
#define PARSE_VERILOG_FILE

#include <unordered_map>
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
    Verilog::Gate ExtractGateData(std::string const &sGateDataFromString); //(Verilog &VerilogModule, FileHandler &VerilogFile);
    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);

    std::unordered_map<std::string, Verilog::Wire>          ExtractWires        (std::string const &sWiresFromString);
    std::unordered_map<std::string, Verilog::PrimaryInput>  ExtractModuleInputs (std::string const &sPortNamesFromString);
    std::unordered_map<std::string, Verilog::PrimaryOutput> ExtractModuleOutputs(std::string const &sPortNamesFromString);
};

#endif
