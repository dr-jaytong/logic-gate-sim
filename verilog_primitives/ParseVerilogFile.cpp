#include <iostream>
#include <vector>
#include <string>

#include "ParseVerilogFile.hpp"
#include "Utilities.hpp"

std::string ParseVerilogFile::ParseNextLine()
{
    bool bStopParsing(false);
    std::string sLineFromFile("");
    std::string sAppendedString("");
    while (!bStopParsing) { 
        sLineFromFile = m_VerilogFile.GetNextLine();
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

void ParseVerilogFile::ExtractGateData()
{
    std::string sLogicGateInfo(m_VerilogFile.GetLastLineParsed());

    if (sLogicGateInfo.find(';') == std::string::npos)
        sLogicGateInfo += ParseNextLine();

    std::string const sGateType (Utility::GetFirstWordFromLine(sLogicGateInfo));
    sLogicGateInfo = Utility::StripString(sLogicGateInfo, sGateType);

    std::string const sGateName (Utility::RemoveWhiteSpace(sLogicGateInfo.substr(0, sLogicGateInfo.find_first_of('('))));
    std::string       sGatePorts(Utility::RemoveWhiteSpace(Utility::StripString(sLogicGateInfo, sGateName)));

    std::cout << "Gate name : \'" << sGateName << "\' type: \'" << sGateType << "\'   sGatePorts: " <<  "\'" << sGatePorts << "\'" << std::endl;

    std::vector<std::string> vGatePorts(ExtractPortNames(sGatePorts));
    for (auto const &sGatePort : vGatePorts)
        std::cout << sGatePort << std::endl;
}

bool ParseVerilogFile::IsGate(std::string const &sKeyword)
{
    return sKeyword == "and" || 
           sKeyword == "nand" ||
           sKeyword == "or" || 
           sKeyword == "nor" || 
           sKeyword == "not";
}

void ParseVerilogFile::ExtractModulePorts()
{
    std::string sModulePorts(m_VerilogFile.GetLastLineParsed());
    if (sModulePorts.find(';') == std::string::npos) // Make sure all the port names are parsed to the ';' character
        sModulePorts += ParseNextLine();

    std::string const sModulePortType(Utility::GetFirstWordFromLine(sModulePorts));
    std::cout << "Found module port type: " << sModulePortType << std::endl;
    sModulePorts = Utility::StripString(sModulePorts, sModulePortType);
    std::vector<std::string> const vModulePorts(ExtractPortNames(sModulePorts));

    std::cout << "Number of " << sModulePortType << " " << vModulePorts.size() << std::endl;
    for (auto const &sPortName : vModulePorts)
        std::cout << "\'" << sPortName << "\' ";
    std::cout << std::endl;
}

void ParseVerilogFile::ExtractVerilogData()
{
    std::string const sKeyword(Utility::GetFirstWordFromLine(m_VerilogFile.GetLastLineParsed()));

    if (sKeyword == "input" || sKeyword == "output" || sKeyword == "wire") {
        std::cout << "Found module port: \'" << sKeyword << "\'" << std::endl;
        ExtractModulePorts();
    }

    if (IsGate(sKeyword)) {
        std::cout << "Found logic gate: " << sKeyword << std::endl;
        ExtractGateData();
    }

}

void ParseVerilogFile::ParseFile()
{
    std::vector<std::string> vTokens;
    while (!m_VerilogFile.eof()) {
        std::string const sLine(m_VerilogFile.GetNextLine());
        
        if (sLine.empty() ||
            sLine.find("module") != std::string::npos ||
            sLine.find("endmodule") != std::string::npos ||
            sLine.find("//") != std::string::npos)
            continue;

        ExtractVerilogData();
    }
}
