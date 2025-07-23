#include "Verilog.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <queue> 

#define CPP_MODULE "VERL"

#include "Logging.hpp"

void Verilog::AddGate(Verilog::Gate const &inputGate) 
{
    for (auto const &sInputPort : inputGate.m_vInputPortNames) {
        auto itFindConnectionFromInput(m_umConnectionID2Connection.find(sInputPort));
        if (itFindConnectionFromInput == m_umConnectionID2Connection.end()) 
            LOG_ERROR("Gate input port \'" + sInputPort + "\" was not found in connections");
   
        itFindConnectionFromInput->second.m_vOutgoingGates.push_back(inputGate.m_sGateIdentifier);
    }

    auto itFindConnectionFromOutput(m_umConnectionID2Connection.find(inputGate.m_sOutputPortName));
    if (itFindConnectionFromOutput == m_umConnectionID2Connection.end())
        LOG_ERROR("Gate output port \'" + inputGate.m_sOutputPortName + "\' was not found in connections");
    
    assert(itFindConnectionFromOutput->second.m_sIncomingGate.empty());
    itFindConnectionFromOutput->second.m_sIncomingGate = inputGate.m_sGateIdentifier;

    if (!m_umGateID2Gates.insert({inputGate.m_sGateIdentifier, std::move(inputGate)}).second) 
        LOG_ERROR(inputGate.m_sGateIdentifier + " already exists !");
}

void Verilog::AddLogic(Verilog::Gate const &inputGate)
{
    inputGate.m_sGateType == "dff" ? AddDFFPorts(inputGate.m_vInputPortNames) : AddGate(inputGate);
}

void Verilog::ConvertModulePort(std::string const &sPort, ConnectionType const eType)
{
    auto itFindConnection(m_umConnectionID2Connection.find(sPort));
    if (itFindConnection == m_umConnectionID2Connection.end())
        LOG_ERROR(sPort + " was not found in connections!");
    itFindConnection->second.m_eType = eType; 
    itFindConnection->second.m_iLevelNumber = eType == ConnectionType::PRIMARY_INPUT ? 0 : -1;
}

void Verilog::AddDFFPorts(std::vector<std::string> const &vDFFPorts) 
{
    assert(vDFFPorts.size() == 2);
    ConvertModulePort(vDFFPorts[0], ConnectionType::PRIMARY_INPUT);
    ConvertModulePort(vDFFPorts[1],  ConnectionType::PRIMARY_OUTPUT);
}

void Verilog::AddConnections(std::unordered_map<std::string, Connection> const &umConnections)
{
    m_umConnectionID2Connection.insert(umConnections.begin(), umConnections.end());
    
    if (umConnections.begin()->second.m_eType == ConnectionType::PRIMARY_INPUT) { // This block is needed for adding outgoing gates from primary inputs during levelization
        std::vector<std::string> vPrimaryInputs(umConnections.size());
        std::transform(umConnections.begin(), umConnections.end(), vPrimaryInputs.begin(), [](auto const &elem) { return elem.first; });
        AddPrimaryInputs(vPrimaryInputs);
    }
}

void Verilog::Levelize()
{
    LOG("Begin Levelization");

    std::queue<std::string> qGatesToAnalyze;
    for (auto const &primaryInput : m_vPrimaryInputs) { // Push all fanout gates from the primary input ports
        for (auto const &sGateID : m_umConnectionID2Connection.at(primaryInput).m_vOutgoingGates) 
            qGatesToAnalyze.push(sGateID);
    }

    while (!qGatesToAnalyze.empty()) {
        std::string const sCurrentGate(qGatesToAnalyze.front());
        qGatesToAnalyze.pop();
    
        int iFoundLargestLevelNumber(-1);
        size_t uNumPortsAnalyzed(0);

        if (m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber != -1) // Skip current gate if it contains a level number 
            continue;

        for (auto const &sInputPort : m_umGateID2Gates.at(sCurrentGate).m_vInputPortNames) {
            auto itFindConnectionFromInput(m_umConnectionID2Connection.find(sInputPort));
            if (itFindConnectionFromInput->second.m_eType == ConnectionType::WIRE) {
                if (itFindConnectionFromInput->second.m_iLevelNumber == -1)
                    break;

                if (itFindConnectionFromInput->second.m_iLevelNumber > iFoundLargestLevelNumber)
                    iFoundLargestLevelNumber = itFindConnectionFromInput->second.m_iLevelNumber;
            }

            if (itFindConnectionFromInput->second.m_eType == ConnectionType::PRIMARY_INPUT) {
                if (itFindConnectionFromInput->second.m_iLevelNumber > iFoundLargestLevelNumber)
                    iFoundLargestLevelNumber = itFindConnectionFromInput->second.m_iLevelNumber;
            }

            ++uNumPortsAnalyzed;
        }

        if (uNumPortsAnalyzed == m_umGateID2Gates.at(sCurrentGate).m_vInputPortNames.size()) { // If all gate's input ports are thoroughly analyzed
            m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber = iFoundLargestLevelNumber + 1; // Assign gate level number
            auto itWire(m_umConnectionID2Connection.find(m_umGateID2Gates.at(sCurrentGate).m_sOutputPortName));
            if (itWire->second.m_eType == ConnectionType::WIRE) {
                itWire->second.m_iLevelNumber = m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber; // Also assign the level number to the net
                for (auto const &sGateID : itWire->second.m_vOutgoingGates) 
                    qGatesToAnalyze.push(sGateID); // Add the fanout gates from the net
            }
        } else {
            qGatesToAnalyze.push(sCurrentGate); // Re-add the current gate because there are still wires with unassigned level numbers
        }
    }
    LOG("Levelization completed");
}

void Verilog::Print() 
{
    std::cout << "Stored Connections " << std::endl;
    for (auto const &PI : m_umConnectionID2Connection) {
        std::cout << "   Connection name: " << PI.first << " Connection Type: " << GetConnectionType(PI.second) << "(" << PI.second.m_eType << ")" << std::endl; 
        std::cout << "   Level : " << PI.second.m_iLevelNumber << std::endl;
        std::cout << "   \t Gate that drives this wire: " << PI.second.m_sIncomingGate << std::endl;
        for (auto const &inputGate : PI.second.m_vOutgoingGates)
            std::cout << "\t Gates rely on this wire: " << inputGate << std::endl;
        std::cout << std::endl;
    }

    std::cout << "Stored Gates " << std::endl;
    for (auto const &gate : m_umGateID2Gates) {
        std::cout << "\t Gate ID: " << gate.second.m_sGateIdentifier << std::endl;
        std::cout << "\t Gate Type: " << gate.second.m_sGateType << std::endl;
        std::cout << "\t Output port: " << gate.second.m_sOutputPortName << std::endl;
        std::cout << "\t input port names: "; 
        for (auto const &inputPort : gate.second.m_vInputPortNames)
            std::cout << inputPort << ", ";
        std::cout <<  std::endl << "\t levelize: " << gate.second.m_iLevelNumber << std::endl;
        std::cout << std::endl << "==== " << std::endl;
    }
}
