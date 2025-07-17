#include <vector>
#include <string>
#include <chrono>

#include "VerilogUtilities.hpp"
#include "Utilities.hpp"

#define CPP_MODULE "PARS"

#include "Logging.hpp"

std::string VerilogUtility::ParseNextVerilogLine(FileHandler &VerilogFile)
{
    bool bStopParsing(false);
    std::string sLineFromFile("");
    std::string sAppendedString("");
    while (!bStopParsing) { 
        sLineFromFile = VerilogFile.GetNextLine();
        bStopParsing = sLineFromFile.find(';') != std::string::npos ? true : false;
        sAppendedString += sLineFromFile;
    }

    return sAppendedString;
}

std::vector<std::string> VerilogUtility::ExtractPortNames(std::string const &sPortsFromString)
{
    std::string const sFilteredLine(Utility::String::Strip(sPortsFromString, std::vector<char>({' ', ';', ')', '('})));
    return Utility::String::Tokenize(sFilteredLine, ',');
}

std::unordered_map<std::string, Verilog::Wire> VerilogUtility::ExtractWires(std::string const &sWiresFromString)
{
    std::vector<std::string> const vWireNames(ExtractPortNames(sWiresFromString));
    std::unordered_map<std::string, Verilog::Wire> umWires;
    for (auto const &wire : vWireNames)
        umWires.insert({wire, Verilog::Wire(wire)});

    return umWires;
}

std::unordered_map<std::string, Verilog::PrimaryInput> VerilogUtility::ExtractModuleInputs(std::string const &sPorts)
{
    std::string const sModulePortType(Utility::String::GetFirstWord(sPorts));
    std::string const sModulePorts(Utility::String::Strip(sPorts, sModulePortType));

    std::unordered_map<std::string, Verilog::PrimaryInput> umPrimaryInputs;
    for (auto const &sPortName : ExtractPortNames(sModulePorts)) 
        umPrimaryInputs.insert({sPortName, Verilog::PrimaryInput(sPortName)});

    return umPrimaryInputs;
}

std::unordered_map<std::string, Verilog::PrimaryOutput> VerilogUtility::ExtractModuleOutputs(std::string const &sPorts)
{
    std::string const sModulePortType(Utility::String::GetFirstWord(sPorts));
    std::string const sModulePorts(Utility::String::Strip(sPorts, sModulePortType));

    std::unordered_map<std::string, Verilog::PrimaryOutput> umPrimaryOutputs;
    for (auto const &sPortName : ExtractPortNames(sModulePorts)) 
        umPrimaryOutputs.insert({sPortName, Verilog::PrimaryOutput(sPortName)});

    return umPrimaryOutputs;
}

Verilog::Gate VerilogUtility::ExtractGateData(std::string const &sGateInfoFromString)
{
    std::string sLogicGateInfo(sGateInfoFromString);

    std::string const sGateType(Utility::String::GetFirstWord(sLogicGateInfo));
    sLogicGateInfo = Utility::String::Strip(sLogicGateInfo, sGateType);

    std::string const sGateName (Utility::String::RemoveWhiteSpace(sLogicGateInfo.substr(0, sLogicGateInfo.find_first_of('('))));
    std::string       sGatePorts(Utility::String::RemoveWhiteSpace(Utility::String::Strip(sLogicGateInfo, sGateName)));

    std::vector<std::string> vGatePorts(ExtractPortNames(sGatePorts));
    return Verilog::Gate(sGateType, sGateName, vGatePorts.front(), std::vector<std::string>(vGatePorts.begin() + 1, vGatePorts.end()));
}

bool VerilogUtility::IsGate(std::string const &sKeyword)
{
    return sKeyword == "and" || 
           sKeyword == "nand" ||
           sKeyword == "or" || 
           sKeyword == "nor" || 
           sKeyword == "not";
}


void VerilogUtility::ParseFile(Verilog &VerilogModule, FileHandler &VerilogFile)
{
    LOG("Parsing and building verilog module structure");
    std::chrono::steady_clock::time_point const tpStartParse(std::chrono::steady_clock::now());

    while (!VerilogFile.eof()) {
        std::string sLine(VerilogFile.GetNextLine());

        if (sLine.empty() ||
            sLine.find("module") != std::string::npos ||
            sLine.find("endmodule") != std::string::npos ||
            sLine.find("//") != std::string::npos)
            continue;

        std::string const sKeyword(Utility::String::GetFirstWord(sLine));

        // Once reached here, verilog line could be either 'input', 'output', 'wire', or gate
        // We have to parse all the way to the next ';'

        if (sKeyword == "input" || sKeyword == "output" || sKeyword == "wire" || IsGate(sKeyword)) {
            if (sLine.find(';') == std::string::npos) 
                sLine += ParseNextVerilogLine(VerilogFile);

            if (sKeyword == "input") 
                VerilogModule.AddInputPorts(ExtractModuleInputs(sLine)); 
            else if (sKeyword == "output") 
                VerilogModule.AddOutputPorts(ExtractModuleOutputs(sLine));
            else if (sKeyword == "wire") 
                VerilogModule.AddWires(ExtractWires(sLine));
            else if (IsGate(sKeyword)) 
                VerilogModule.AddGate(ExtractGateData(sLine));
            
        }
    }
    std::chrono::steady_clock::time_point const tpStopParse(std::chrono::steady_clock::now());
    LOG("Parse and construction completed in " << Utility::PrintElapsedTime(tpStopParse, tpStartParse));
}
