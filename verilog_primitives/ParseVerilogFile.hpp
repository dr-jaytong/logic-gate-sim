#ifndef PARSE_VERILOG_FILE
#define PARSE_VERILOG_FILE

#include <unordered_map>
#include <vector>
#include <string>

#include "Verilog.hpp"
#include "FileHandler.hpp"

namespace ParseVerilogFile
{
    std::string ParseNextLine(FileHandler &VerilogFile);
    bool IsGate(std::string const &sLine);
    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);
    std::unordered_map<std::string, Verilog::Wire> ExtractWires(std::string const &sWiresFromString);
    void ParseFile(Verilog &VerilogModule, FileHandler &VerilogFile);
    Verilog::Gate ExtractGateData(std::string const &sGateDataFromString); //(Verilog &VerilogModule, FileHandler &VerilogFile);
    void ExtractVerilogData(Verilog &VerilogModule, FileHandler &VerilogFile);
    std::vector<std::string> ExtractModulePorts(std::string const &sPorts); //FileHandler &VerilogFile);
};

#endif
