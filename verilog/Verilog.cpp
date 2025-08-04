#include "Verilog.hpp"
#include "Utilities.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <queue> 
#include <format>
#include <ranges>

#define CPP_MODULE "VERL"

#include "Logging.hpp"

bool Verilog::IsGate(std::string const &sKeyword)
{
    return sKeyword == "and"  || 
           sKeyword == "nand" ||
           sKeyword == "or"   || 
           sKeyword == "nor"  || 
           sKeyword == "not"  ||
           sKeyword == "xor"  ||
           sKeyword == "dff";
}

// TODO: Use input ports for each gate type (e.g., and2, and3, and4, .etc)

std::bitset<8> const Verilog::GetGateEncoding(std::string const &sGateType) 
{
    return sGateType == "and"  ? 0b00000001 : 
                        "nand" ? 0b00000010 :
                        "or"   ? 0b00000100 :
                        "nor"  ? 0b00001000 :
                        "not"  ? 0b00010000 :
                        "xor"  ? 0b00100000 :
                                 0b11111111 ; // Should not exist

}

void Verilog::AddGate(Verilog::Gate const &inputGate) 
{
    // First, find gate's output if it is part of the primary output connection
    auto itFindConnectionFromOutput(m_ModuleData.umConnectionID2Connection.find(inputGate.m_sOutputPortName));
    if (itFindConnectionFromOutput != m_ModuleData.umConnectionID2Connection.end()) {
        if (itFindConnectionFromOutput->second.m_eType == ConnectionType::PRIMARY_OUTPUT || 
            itFindConnectionFromOutput->second.m_eType == ConnectionType::PRIMARY_OUTPUT_DFF) {
            assert(itFindConnectionFromOutput->second.m_sIncomingGate.empty());
            itFindConnectionFromOutput->second.m_sIncomingGate = inputGate.m_sGateIdentifier;
        } else {
            LOG_ERROR("Fatal error! " + itFindConnectionFromOutput->first + " was already created but not a primary output!");
        }
    } else {
        m_ModuleData.umConnectionID2Connection.insert({inputGate.m_sOutputPortName, Verilog::Connection(inputGate.m_sOutputPortName, Verilog::ConnectionType::WIRE, 0)});
        m_ModuleData.umConnectionID2Connection.at(inputGate.m_sOutputPortName).m_sIncomingGate = inputGate.m_sGateIdentifier;
    }

    if (!m_ModuleData.umGateID2Gates.insert({inputGate.m_sGateIdentifier, std::move(inputGate)}).second) 
        LOG_ERROR(inputGate.m_sGateIdentifier + " already exists !");
}

void Verilog::ResolveWireFanouts()
{
    for (auto const &Gate : m_ModuleData.umGateID2Gates) {
        for (auto const &port : Gate.second.m_vInputPortNames) {
            auto itFindConnectionFromInput(m_ModuleData.umConnectionID2Connection.find(port));
            if (itFindConnectionFromInput == m_ModuleData.umConnectionID2Connection.end())
                LOG_ERROR("Fatal error, Gate's input port: " + port + " was not found in connections!");

            itFindConnectionFromInput->second.m_vOutgoingGates.push_back(Gate.second.m_sGateIdentifier);
        }
    }
}


void Verilog::AddLogic(Verilog::Gate const &inputGate)
{
    inputGate.m_sGateType == "dff" ? AddDFFPorts(inputGate.m_vInputPortNames) : AddGate(inputGate);
}

bool Verilog::ConvertModulePort(std::string const &sPort, ConnectionType const eType)
{
    return m_ModuleData.umConnectionID2Connection.insert({sPort, Verilog::Connection(sPort, eType, 0)}).second;
}

void Verilog::AddDFFPorts(std::vector<std::string> const &vDFFPorts) 
{
    assert(vDFFPorts.size() == 2);
    if (!ConvertModulePort(vDFFPorts[0], ConnectionType::PRIMARY_INPUT_DFF))
        LOG_WARNING("Unable to convert module port " + vDFFPorts[0] + " as primary input" );
    m_ModuleData.vPrimaryInputs.push_back(vDFFPorts[0]);
    
    if (!ConvertModulePort(vDFFPorts[1],  ConnectionType::PRIMARY_OUTPUT_DFF))
        LOG_WARNING("Unable to convert module port " + vDFFPorts[1] + " as primary output");
}

void Verilog::AddConnections(std::unordered_map<std::string, Connection> const &umConnections)
{
    m_ModuleData.umConnectionID2Connection.insert(umConnections.begin(), umConnections.end());
    
    if (umConnections.begin()->second.m_eType == ConnectionType::PRIMARY_INPUT) { // This block is needed for adding outgoing gates from primary inputs during levelization
        std::vector<std::string> vPrimaryInputs(umConnections.size());
        std::transform(umConnections.begin(), umConnections.end(), vPrimaryInputs.begin(), [](auto const &elem) { return elem.first; });
        AddPrimaryInputs(vPrimaryInputs);
    }
}

std::vector<std::string> Verilog::ExtractPortNames(std::string const &sPortsFromString)
{
    std::string const sFilteredLine(Utility::String::Strip(Utility::String::RemoveWhiteSpace(sPortsFromString), std::vector<char>({' ', ';', ')', '('})));
    return Utility::String::Tokenize(sFilteredLine, ',');
}

std::unordered_map<std::string, Verilog::Connection> Verilog::ExtractConnections(std::string const &sNamesFromString, int &iNetAddress)
{
    std::string const sConnectionType(Utility::String::GetFirstWord(sNamesFromString));
    std::vector<std::string> const vConnectionNames(ExtractPortNames(Utility::String::Strip(sNamesFromString, sConnectionType)));
    std::unordered_map<std::string, Verilog::Connection> umConnections;
    for (auto const &sName : vConnectionNames) {
        if (Utility::String::ToLowerCase(sName) == "ck" || Utility::String::ToLowerCase(sName) == "clk") {
            /////std::cout << "Skipping clock signal" << std::endl;
            continue;
        }

        umConnections.insert({sName, sConnectionType == "input"  ? Verilog::Connection(sName, Verilog::ConnectionType::PRIMARY_INPUT, iNetAddress) :
                                     sConnectionType == "output" ? Verilog::Connection(sName, Verilog::ConnectionType::PRIMARY_OUTPUT, iNetAddress) : 
                                                                   Verilog::Connection(sName, Verilog::ConnectionType::WIRE, iNetAddress)});
        ++iNetAddress;
    }

    return umConnections;
}

Verilog::Gate Verilog::ExtractLogicData(std::string const &sGateInfoFromString)
{
    // Gate level netlist is in the form of: gate_type gate_ID(output_net, input_net1, input_net2, ....);

    std::string const sGateType     (Utility::String::GetFirstWord(sGateInfoFromString)); 
    std::string const sLogicGateInfo(Utility::String::Strip(sGateInfoFromString, sGateType)); 
    std::string const sGateName     (Utility::String::RemoveWhiteSpace(sLogicGateInfo.substr(0, sLogicGateInfo.find_first_of('('))));
    std::string const sGatePorts    (Utility::String::RemoveWhiteSpace(Utility::String::Strip(sLogicGateInfo, sGateName)));

    if (m_ModuleData.umGateType2TotalInstance.find(sGateType) == m_ModuleData.umGateType2TotalInstance.end())
        m_ModuleData.umGateType2TotalInstance.insert({sGateType, 0});

    m_ModuleData.umGateType2TotalInstance.at(sGateType) += 1;

    std::vector<std::string> vGatePorts(ExtractPortNames(sGatePorts));
    return Verilog::Gate(sGateType, sGateName, vGatePorts.front(), std::vector<std::string>(vGatePorts.begin() + 1, vGatePorts.end()));
}

Verilog::PQLevel2GateID Verilog::Levelize()
{
    LOG("Begin Levelization");

    PQLevel2GateID pqLevelizedGates;
    std::queue<std::string> qGatesToAnalyze;
    for (auto const &primaryInput : m_ModuleData.vPrimaryInputs) { // Push all fanout gates from the primary input ports
        for (auto const &sGateID : m_ModuleData.umConnectionID2Connection.at(primaryInput).m_vOutgoingGates) 
            qGatesToAnalyze.push(sGateID);
    }

    while (!qGatesToAnalyze.empty()) {
        std::string const sCurrentGate(qGatesToAnalyze.front());
        qGatesToAnalyze.pop();
   
        /////std::cout << "Analyzing current gate: " << m_ModuleData.umGateID2Gates.at(sCurrentGate) << std::endl;

        int iFoundLargestLevelNumber(-1);
        size_t uNumPortsAnalyzed(0);

        if (m_ModuleData.umGateID2Gates.at(sCurrentGate).m_iLevelNumber != -1) // Skip current gate if it contains a level number 
            continue;

        for (auto const &sInputPort : m_ModuleData.umGateID2Gates.at(sCurrentGate).m_vInputPortNames) {
            auto itFindConnectionFromInput(m_ModuleData.umConnectionID2Connection.find(sInputPort));
            if (itFindConnectionFromInput->second.m_eType == ConnectionType::WIRE) {
                ////std::cout << "sInputPort: " << sInputPort << " is wire connection ";
                if (itFindConnectionFromInput->second.m_iLevelNumber == -1) {
                    ////std::cout << " not assigned to a level number!" << std::endl;
                    break;
                }

                if (itFindConnectionFromInput->second.m_iLevelNumber > iFoundLargestLevelNumber) {
                    ////std::cout << " has level number: " << itFindConnectionFromInput->second.m_iLevelNumber << std::endl;
                    iFoundLargestLevelNumber = itFindConnectionFromInput->second.m_iLevelNumber;
                }
            }

            if (itFindConnectionFromInput->second.m_eType == ConnectionType::PRIMARY_INPUT) {
                ////std::cout << "sInputPort: " << sInputPort << " is a primary INPUT " << std::endl;
                if (itFindConnectionFromInput->second.m_iLevelNumber > iFoundLargestLevelNumber)
                    iFoundLargestLevelNumber = itFindConnectionFromInput->second.m_iLevelNumber;
            }

            ++uNumPortsAnalyzed;
        }

        if (uNumPortsAnalyzed == m_ModuleData.umGateID2Gates.at(sCurrentGate).m_vInputPortNames.size()) { // If all gate's input ports are thoroughly analyzed
            m_ModuleData.umGateID2Gates.at(sCurrentGate).m_iLevelNumber = iFoundLargestLevelNumber + 1; // Assign gate level number
            pqLevelizedGates.push(Level_2_GateID(m_ModuleData.umGateID2Gates.at(sCurrentGate).m_iLevelNumber, sCurrentGate)); ; // Create a copy
            ////std::cout << "ASSIGNING GATE: " << sCurrentGate << " to level number: " << iFoundLargestLevelNumber + 1 << std::endl;

            auto itWire(m_ModuleData.umConnectionID2Connection.find(m_ModuleData.umGateID2Gates.at(sCurrentGate).m_sOutputPortName));
            ////std::cout << "FOUND CONNECTION: " << itWire->second << std::endl;
            ////if (itWire->second.m_eType == ConnectionType::WIRE) {
                itWire->second.m_iLevelNumber = m_ModuleData.umGateID2Gates.at(sCurrentGate).m_iLevelNumber; // Also assign the level number to the net
                ////std::cout << "Assigning CONNECTION: " << itWire->second << std::endl;                                                                                       
                for (auto const &sGateID : itWire->second.m_vOutgoingGates) { 
                    qGatesToAnalyze.push(sGateID); // Add the fanout gates from the net
                    ////std::cout << "Pushing next gate: " << sGateID << std::endl;
                }
            ////}
        } else {
            ////std::cout << "Readding gate: " << sCurrentGate << std::endl;
            qGatesToAnalyze.push(sCurrentGate); // Re-add the current gate because there are still wires with unassigned level numbers
        }
    }

    LOG("Levelization completed");
    return pqLevelizedGates;
}

std::string Verilog::ParseNextVerilogLine(FileHandler &VerilogFile)
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

void Verilog::ParseFile(FileHandler &&VerilogFile)
{
    LOG("Parsing and building verilog module structure");
    std::chrono::steady_clock::time_point const tpStartParse(std::chrono::steady_clock::now());

    int iNetAddress(0);

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

        if (sKeyword == "input" || sKeyword == "output" || /*sKeyword == "wire" || */ Verilog::IsGate(sKeyword)) {
            if (sLine.find(';') == std::string::npos) 
                sLine += ParseNextVerilogLine(VerilogFile);

            IsGate(sKeyword) ? AddLogic(ExtractLogicData(sLine)) : AddConnections(ExtractConnections(sLine, iNetAddress));
        }
    }
    std::chrono::steady_clock::time_point const tpStopParse(std::chrono::steady_clock::now());
    LOG("Parse and construction completed in " << Utility::PrintElapsedTime(tpStopParse, tpStartParse));


    ResolveWireFanouts();
}

void Verilog::GenerateGateData(PQLevel2GateID &&pqLevelizedGates)
{
    m_EncodedData.vNets.resize(m_ModuleData.umConnectionID2Connection.size(), 0);

    while (!pqLevelizedGates.empty()) {
        ////std::cout << pqLevelizedGates.top().first << ", " << pqLevelizedGates.top().second << std::endl; 
        std::string const sGateID(pqLevelizedGates.top().second);
        m_EncodedData.vGates.emplace_back(GetGateEncoding(m_ModuleData.umGateID2Gates.at(sGateID).m_sGateType));

        // TODO: Include port/net addresses
        pqLevelizedGates.pop();
    }

    /////for (auto const &GateCode : m_EncodedData.vGates)
    /////    std::cout << GateCode << std::endl;

}

void Verilog::BuildModule(std::string const &sFileName)
{
    ParseFile(FileHandler(sFileName));
    GenerateGateData(Levelize());
}

void Verilog::PrintModuleStats() 
{
    LOG(std::format("{:>10} | {:^10}", "Gate Type", "Total")); // Header file
    LOG(std::format("{:*>10} | {:*^10}", "", ""));

    for (auto const &elem : m_ModuleData.umGateType2TotalInstance) {
        LOG(std::format("{:>10} | {:>10}  ", elem.first, elem.second));
    }
}
