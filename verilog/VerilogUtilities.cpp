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

std::unordered_map<std::string, Verilog::Connection> VerilogUtility::ExtractConnections(std::string const &sNamesFromString)
{
    std::string const sConnectionType(Utility::String::GetFirstWord(sNamesFromString));
    std::vector<std::string> const vConnectionNames(ExtractPortNames(Utility::String::Strip(sNamesFromString, sConnectionType)));
    std::unordered_map<std::string, Verilog::Connection> umConnections;
    for (auto const &sName : vConnectionNames) {
        if (Utility::String::ToLowerCase(sName) == "ck" || Utility::String::ToLowerCase(sName) == "clk") {
            std::cout << "Skipping clock signal" << std::endl;
            continue;
        }

        umConnections.insert({sName, sConnectionType == "input"  ? Verilog::Connection(sName, Verilog::ConnectionType::PRIMARY_INPUT) :
                                     sConnectionType == "output" ? Verilog::Connection(sName, Verilog::ConnectionType::PRIMARY_OUTPUT) : 
                                                                   Verilog::Connection(sName, Verilog::ConnectionType::WIRE)});
    }

    return umConnections;
}

Verilog::Gate VerilogUtility::ExtractLogicData(std::string const &sGateInfoFromString)
{
    // Gate level netlist is in the form of: gate_type gate_ID(output_net, input_net1, input_net2, ....);

    std::string const sGateType     (Utility::String::GetFirstWord(sGateInfoFromString)); 
    std::string const sLogicGateInfo(Utility::String::Strip(sGateInfoFromString, sGateType)); 
    std::string const sGateName     (Utility::String::RemoveWhiteSpace(sLogicGateInfo.substr(0, sLogicGateInfo.find_first_of('('))));
    std::string const sGatePorts    (Utility::String::RemoveWhiteSpace(Utility::String::Strip(sLogicGateInfo, sGateName)));

    std::vector<std::string> vGatePorts(ExtractPortNames(sGatePorts));
    return Verilog::Gate(sGateType, sGateName, vGatePorts.front(), std::vector<std::string>(vGatePorts.begin() + 1, vGatePorts.end()));
}

bool VerilogUtility::IsGate(std::string const &sKeyword)
{
    return sKeyword == "and"  || 
           sKeyword == "nand" ||
           sKeyword == "or"   || 
           sKeyword == "nor"  || 
           sKeyword == "not"  ||
           sKeyword == "xor"  ||
           sKeyword == "dff";
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

            IsGate(sKeyword) ? VerilogModule.AddLogic(ExtractLogicData(sLine)) : VerilogModule.AddConnections(ExtractConnections(sLine));
        }
    }
    std::chrono::steady_clock::time_point const tpStopParse(std::chrono::steady_clock::now());
    LOG("Parse and construction completed in " << Utility::PrintElapsedTime(tpStopParse, tpStartParse));
}
