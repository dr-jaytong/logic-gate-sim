#include "Verilog.hpp"
#include "Utilities.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <queue> 

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
    for (auto const &sInputPort : inputGate.m_vInputPortNames) {
        auto itFindConnectionFromInput(m_ModuleData.umConnectionID2Connection.find(sInputPort));
        if (itFindConnectionFromInput == m_ModuleData.umConnectionID2Connection.end()) 
            LOG_ERROR("Gate input port \'" + sInputPort + "\" was not found in connections");
   
        itFindConnectionFromInput->second.m_vOutgoingGates.push_back(inputGate.m_sGateIdentifier);
    }

    auto itFindConnectionFromOutput(m_ModuleData.umConnectionID2Connection.find(inputGate.m_sOutputPortName));
    if (itFindConnectionFromOutput == m_ModuleData.umConnectionID2Connection.end())
        LOG_ERROR("Gate output port \'" + inputGate.m_sOutputPortName + "\' was not found in connections");
    
    assert(itFindConnectionFromOutput->second.m_sIncomingGate.empty());
    itFindConnectionFromOutput->second.m_sIncomingGate = inputGate.m_sGateIdentifier;

    if (!m_ModuleData.umGateID2Gates.insert({inputGate.m_sGateIdentifier, std::move(inputGate)}).second) 
        LOG_ERROR(inputGate.m_sGateIdentifier + " already exists !");
}

void Verilog::AddLogic(Verilog::Gate const &inputGate)
{
    inputGate.m_sGateType == "dff" ? AddDFFPorts(inputGate.m_vInputPortNames) : AddGate(inputGate);
}

void Verilog::ConvertModulePort(std::string const &sPort, ConnectionType const eType)
{
    auto itFindConnection(m_ModuleData.umConnectionID2Connection.find(sPort));
    if (itFindConnection == m_ModuleData.umConnectionID2Connection.end())
        LOG_ERROR(sPort + " was not found in connections!");
    itFindConnection->second.m_eType = eType; 
    itFindConnection->second.m_iLevelNumber = eType == ConnectionType::PRIMARY_INPUT ? 0 : -1;
}

void Verilog::AddDFFPorts(std::vector<std::string> const &vDFFPorts) 
{
    assert(vDFFPorts.size() == 2);
    ConvertModulePort(vDFFPorts[0], ConnectionType::PRIMARY_INPUT);
    m_ModuleData.vPrimaryInputs.push_back(vDFFPorts[0]);
    ConvertModulePort(vDFFPorts[1],  ConnectionType::PRIMARY_OUTPUT);
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
    std::string const sFilteredLine(Utility::String::Strip(sPortsFromString, std::vector<char>({' ', ';', ')', '('})));
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
   
        ///std::cout << "Analyzing current gate: " << m_ModuleData.umGateID2Gates.at(sCurrentGate) << std::endl;

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
            /////std::cout << "ASSIGNING GATE: " << sCurrentGate << " to level number: " << iFoundLargestLevelNumber + 1 << std::endl;

            auto itWire(m_ModuleData.umConnectionID2Connection.find(m_ModuleData.umGateID2Gates.at(sCurrentGate).m_sOutputPortName));
            if (itWire->second.m_eType == ConnectionType::WIRE) {
                itWire->second.m_iLevelNumber = m_ModuleData.umGateID2Gates.at(sCurrentGate).m_iLevelNumber; // Also assign the level number to the net
                /////std::cout << "Assigning WIRE: " << itWire->first << std::endl;                                                                                            
                for (auto const &sGateID : itWire->second.m_vOutgoingGates) { 
                    qGatesToAnalyze.push(sGateID); // Add the fanout gates from the net
                    /////std::cout << "Pushing next gate: " << sGateID << std::endl;
                }
            }
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

        if (sKeyword == "input" || sKeyword == "output" || sKeyword == "wire" || Verilog::IsGate(sKeyword)) {
            if (sLine.find(';') == std::string::npos) 
                sLine += ParseNextVerilogLine(VerilogFile);

            IsGate(sKeyword) ? AddLogic(ExtractLogicData(sLine)) : AddConnections(ExtractConnections(sLine, iNetAddress));
        }
    }
    std::chrono::steady_clock::time_point const tpStopParse(std::chrono::steady_clock::now());
    LOG("Parse and construction completed in " << Utility::PrintElapsedTime(tpStopParse, tpStartParse));
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

void Verilog::Print() 
{
    std::cout << "Stored Connections " << std::endl;
    for (auto const &connection : m_ModuleData.umConnectionID2Connection)
        LOG(connection.second);

    std::cout << "Stored Gates " << std::endl;
    for (auto const &gate :m_ModuleData.umGateID2Gates)
        LOG(gate.second);
}
