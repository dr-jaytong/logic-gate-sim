#include <vector>
#include <string>

#include "ParseVerilogFile.hpp"
#include "Utilities.hpp"

std::string ParseVerilogFile::ParseNextLine(FileHandler &VerilogFile)
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

std::vector<std::string> ParseVerilogFile::ExtractPortNames(std::string const &sPortsFromString)
{
    std::string const sFilteredLine(Utility::StripString(sPortsFromString, std::vector<char>({' ', ';', ')', '('})));
    return Utility::TokenizeString(sFilteredLine, ',');
}

std::unordered_map<std::string, Verilog::Wire> ParseVerilogFile::ExtractWires(std::string const &sWiresFromString)
{
    std::vector<std::string> const vWireNames(ExtractPortNames(sWiresFromString));
    std::unordered_map<std::string, Verilog::Wire> umWires;
    for (auto const &wire : vWireNames)
        umWires.insert({wire, Verilog::Wire(wire)});

    return umWires;
}

std::unordered_map<std::string, Verilog::PrimaryInput> ParseVerilogFile::ExtractModuleInputs(std::string const &sPorts)
{
    std::string const sModulePortType(Utility::GetFirstWordFromLine(sPorts));
    std::string const sModulePorts(Utility::StripString(sPorts, sModulePortType));

    std::unordered_map<std::string, Verilog::PrimaryInput> umPrimaryInputs;
    for (auto const &sPortName : ExtractPortNames(sModulePorts)) 
        umPrimaryInputs.insert({sPortName, Verilog::PrimaryInput(sPortName)});

    return umPrimaryInputs;
}

std::unordered_map<std::string, Verilog::PrimaryOutput> ParseVerilogFile::ExtractModuleOutputs(std::string const &sPorts)
{
    std::string const sModulePortType(Utility::GetFirstWordFromLine(sPorts));
    std::string const sModulePorts(Utility::StripString(sPorts, sModulePortType));

    std::unordered_map<std::string, Verilog::PrimaryOutput> umPrimaryOutputs;
    for (auto const &sPortName : ExtractPortNames(sModulePorts)) 
        umPrimaryOutputs.insert({sPortName, Verilog::PrimaryOutput(sPortName)});

    return umPrimaryOutputs;
}

Verilog::Gate ParseVerilogFile::ExtractGateData(std::string const &sGateInfoFromString)
{
    std::string sLogicGateInfo(sGateInfoFromString);

    std::string const sGateType(Utility::GetFirstWordFromLine(sLogicGateInfo));
    sLogicGateInfo = Utility::StripString(sLogicGateInfo, sGateType);

    std::string const sGateName (Utility::RemoveWhiteSpace(sLogicGateInfo.substr(0, sLogicGateInfo.find_first_of('('))));
    std::string       sGatePorts(Utility::RemoveWhiteSpace(Utility::StripString(sLogicGateInfo, sGateName)));

    std::vector<std::string> vGatePorts(ExtractPortNames(sGatePorts));
    return Verilog::Gate(sGateType, sGateName, vGatePorts.front(), std::vector<std::string>(vGatePorts.begin() + 1, vGatePorts.end()));
}

bool ParseVerilogFile::IsGate(std::string const &sKeyword)
{
    return sKeyword == "and" || 
           sKeyword == "nand" ||
           sKeyword == "or" || 
           sKeyword == "nor" || 
           sKeyword == "not";
}


void ParseVerilogFile::ParseFile(Verilog &VerilogModule, FileHandler &VerilogFile)
{
    while (!VerilogFile.eof()) {
        std::string sLine(VerilogFile.GetNextLine());

        if (sLine.empty() ||
            sLine.find("module") != std::string::npos ||
            sLine.find("endmodule") != std::string::npos ||
            sLine.find("//") != std::string::npos)
            continue;

        std::string const sKeyword(Utility::GetFirstWordFromLine(sLine));

        // Once reached here, verilog line could be either 'input', 'output', 'wire', or gate
        // We have to parse all the way to the next ';'

        if (sKeyword == "input" || sKeyword == "output" || sKeyword == "wire" || IsGate(sKeyword)) {
            if (sLine.find(';') == std::string::npos) 
                sLine += ParseNextLine(VerilogFile);

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
}
