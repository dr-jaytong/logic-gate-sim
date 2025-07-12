#include <iostream>
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

std::vector<std::string> ParseVerilogFile::ExtractPortNames(FileHandler &VerilogFile)
{
    return ExtractPortNames(VerilogFile, "");
}

std::vector<std::string> ParseVerilogFile::ExtractPortNames(FileHandler &VerilogFile, std::string const &sPortsFromString)
{
    std::string sInputPorts(!sPortsFromString.empty() ? sPortsFromString : VerilogFile.GetLastLineParsed());
    std::string sLineFromFile("");

    if (sInputPorts.find(';') == std::string::npos)
        sInputPorts += ParseNextLine(VerilogFile);

    std::string sFilteredLine(sInputPorts);
    std::string sWireType("");

    if (sPortsFromString.empty()) { 
        sWireType = Utilities::GetFirstWordFromLine(sFilteredLine);
        std::cout << "Found : \'" << sWireType << "\' wire type " << std::endl;
        sFilteredLine = Utilities::StripString(sFilteredLine, sWireType); // Remove type input / wire / output
    }

    sFilteredLine = Utilities::StripString(sFilteredLine, std::vector<char>({' ', ';', ')', '('}));
    return Utilities::TokenizeString(sFilteredLine, ',');
}

void ParseVerilogFile::ExtractGate(FileHandler &VerilogFile, std::string const &sFirstLine)
{
    std::string const sGateType(Utilities::GetFirstWordFromLine(sFirstLine));
    std::string sGateIDAndPorts(Utilities::StripString(sFirstLine, sGateType));

    if (sFirstLine.find(';') == std::string::npos)
        sGateIDAndPorts += ParseNextLine(VerilogFile);

    std::string const sGateName (Utilities::RemoveWhiteSpace(sGateIDAndPorts.substr(0, sGateIDAndPorts.find_first_of('('))));
    std::string       sGatePorts(Utilities::RemoveWhiteSpace(Utilities::StripString(sGateIDAndPorts, sGateName)));

    std::cout << "Gate name : \'" << sGateName << "\' type: \'" << sGateType << "\'   sGatePorts: " <<  "\'" << sGatePorts << "\'" << std::endl;

    std::vector<std::string> vGatePorts(ExtractPortNames(VerilogFile, sGatePorts));
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

void ParseVerilogFile::ParseFile(std::string const &sFileName)
{
    FileHandler VerilogFile(sFileName);
    std::vector<std::string> vTokens;
    while (!VerilogFile.eof()) {
        std::string sLine(VerilogFile.GetNextLine());

        if (sLine.empty())
            continue;

        std::string const sKeyword(Utilities::GetFirstWordFromLine(sLine));
        if (sKeyword == "//" || sKeyword == "module" || sKeyword == "endmodule")
            continue;


        if (sKeyword == "input") { 
            std::vector<std::string> const vInputPorts(ExtractPortNames(VerilogFile));
            std::cout << "Found " << vInputPorts.size() << " inputs " << std::endl;
        }

        if (sKeyword == "wire") {
            std::vector<std::string> const vWires(ExtractPortNames(VerilogFile));
            std::cout << "Found " << vWires.size() << " wires " << std::endl;
        }

        if (sKeyword == "output") {
            std::vector<std::string> const vOutputPorts(ExtractPortNames(VerilogFile));
            std::cout << "Found " << vOutputPorts.size() << " outputs " << std::endl;
        }

        if (IsGate(sKeyword)) {
            std::cout << "found a gate: " << sKeyword << std::endl;
            ExtractGate(VerilogFile, sLine);
        }
    }
}
