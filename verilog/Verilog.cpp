#include "Verilog.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>
#include <queue> 

#define CPP_MODULE "VERL"

#include "Logging.hpp"

void Verilog::AddGate(Verilog::Gate const &inputGate) 
{
    std::unordered_map<std::string, Verilog::Connection>::iterator itFindWire;
    for (auto const &sInputPort : inputGate.m_vInputPortNames) {
        itFindWire = m_umWireName2GateOutput.find(sInputPort);
        if (itFindWire != m_umWireName2GateOutput.end()) 
            itFindWire->second.m_vOutgoingGates.push_back(inputGate.m_sGateIdentifier);
    
        std::unordered_map<std::string, Verilog::Connection>::iterator itFindPI(m_umInputPorts.find(sInputPort));
        if (itFindPI != m_umInputPorts.end())
            itFindPI->second.m_vOutgoingGates.push_back(inputGate.m_sGateIdentifier);
    }
    
    itFindWire = m_umWireName2GateOutput.find(inputGate.m_sOutputPortName);
    if (itFindWire != m_umWireName2GateOutput.end()) {
        assert(itFindWire->second.m_sIncomingGate.empty());
        itFindWire->second.m_sIncomingGate = inputGate.m_sGateIdentifier;
    }

    std::unordered_map<std::string, Verilog::Connection>::iterator itFindPO(m_umOutputPorts.find(inputGate.m_sOutputPortName));
    if (itFindPO != m_umOutputPorts.end()) {
        assert(itFindPO->second.m_sIncomingGate.empty()); // Make sure only one gate drives the PO
        itFindPO->second.m_sIncomingGate = inputGate.m_sGateIdentifier;
    }

    if (!m_umGateID2Gates.insert({inputGate.m_sGateIdentifier, std::move(inputGate)}).second) 
        throw std::runtime_error(inputGate.m_sGateIdentifier + " already exists !");
}

void Verilog::Levelize()
{
    LOG("Begin Levelization");

    std::queue<std::string> setGatesToAnalyze;
    for (auto const &primaryInput : m_umInputPorts) { // Push all fanout gates from the primary input ports
        for (auto const &sGateID : primaryInput.second.m_vOutgoingGates) 
            setGatesToAnalyze.push(sGateID);
    }

    while (!setGatesToAnalyze.empty()) {
        std::string const sCurrentGate(setGatesToAnalyze.front());
        setGatesToAnalyze.pop();//erase(setGatesToAnalyze.begin());
    
        std::cout << "[CURRENT GATE: ] " << sCurrentGate << std::endl;

        int iFoundLargestLevelNumber(-1);
        size_t uNumPortsAnalyzed(0);

        if (m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber != -1) {
            std::cout << "GATE WAS ALREADY ASSIGNED! SKIPPING!";
            continue;
        }


        for (auto const &sInputPort : m_umGateID2Gates.at(sCurrentGate).m_vInputPortNames) {
            std::cout << "analyzing port: " << sInputPort << " from gate: " << sCurrentGate <<  std::endl;
            std::unordered_map<std::string, Verilog::Connection>::iterator itFoundPrimaryInput(m_umInputPorts.find(sInputPort)), itFoundWire(m_umWireName2GateOutput.find(sInputPort));
           
            if (itFoundWire != m_umWireName2GateOutput.end() && itFoundWire->second.m_iLevelNumber == -1) {
                std::cout << "Wire : " << itFoundWire->first << " was not assigned" << std::endl;
                break;
            }

            if (itFoundWire != m_umWireName2GateOutput.end()) {
                std::cout << "\t It's a wire with level number : " << itFoundWire->second.m_iLevelNumber << std::endl;
                if (itFoundWire->second.m_iLevelNumber > iFoundLargestLevelNumber) 
                    iFoundLargestLevelNumber = itFoundWire->second.m_iLevelNumber;
            } else {
                std::cout << "\t NOT FOUND in wire" << std::endl;
            }

            if (itFoundPrimaryInput != m_umInputPorts.end()) {
                std::cout << "\t It's a primaryInput with level number : " << itFoundPrimaryInput->second.m_iLevelNumber << std::endl;
                if (itFoundPrimaryInput->second.m_iLevelNumber > iFoundLargestLevelNumber)
                    iFoundLargestLevelNumber = itFoundPrimaryInput->second.m_iLevelNumber;
            } else {
                std::cout << "\t NOT FOUND in PRIMARY INPUTS" << std::endl;
            }
            ++uNumPortsAnalyzed;
        }

        if (uNumPortsAnalyzed == m_umGateID2Gates.at(sCurrentGate).m_vInputPortNames.size()) {
            m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber = iFoundLargestLevelNumber + 1; // Assign gate level number
            std::cout << "\t\t\t Gate: " << sCurrentGate << " has level number : " << m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber << std::endl;

            std::cout << "Assigning new gates to analyze" << std::endl;
            std::unordered_map<std::string, Verilog::Connection>::iterator itWire(m_umWireName2GateOutput.find(m_umGateID2Gates.at(sCurrentGate).m_sOutputPortName));
            if (itWire != m_umWireName2GateOutput.end()) {
                std::cout << "\t Output port : " << itWire->first << " is NOT a primary output" << std::endl;
                itWire->second.m_iLevelNumber = m_umGateID2Gates.at(sCurrentGate).m_iLevelNumber;
                std::cout << "\t Assigning wire : " << itWire->first << " level number: " << itWire->second.m_iLevelNumber << std::endl;
                for (auto const &sGateID : itWire->second.m_vOutgoingGates) {
                    std::cout << "Adding NEW GATE TO ANALYZE: " << sGateID << std::endl;
                    setGatesToAnalyze.push(sGateID);
                }
            }
        } else {
            setGatesToAnalyze.push(sCurrentGate);
        }
    }
    LOG("Levelization completed");
}



void Verilog::Print() 
{
    std::cout << "Stored Primary Inputs " << std::endl;
    for (auto const &PI : m_umInputPorts) {
        std::cout << "Primary input: \'" << PI.first << "\' ";
        std::cout << "Stored depending gates on PI : ";
        for (auto const &inputPort : PI.second.m_vOutgoingGates)
            std::cout << inputPort << " ";
        std::cout << std::endl;
    }

    std::cout << "Stored Primary Outputs " << std::endl;
    for (auto const &PO : m_umOutputPorts) {
        std::cout << "Primary output: \'" << PO.first << "\' " << std::endl;
        std::cout << "Stored gate driving the PO: ";
        std::cout << PO.second.m_sIncomingGate << std::endl;
    }

    std::cout << "Stored Wires " << std::endl;
    for (auto const &PI : m_umWireName2GateOutput) {
        std::cout << "   Wire name: " << PI.first << "   Level : " << PI.second.m_iLevelNumber << std::endl;
        std::cout << "   \t Gate that drives this wire: " << PI.second.m_sIncomingGate << std::endl;
        for (auto const &inputGate : PI.second.m_vOutgoingGates)
            std::cout << "\t Gates rely on this wire: " << inputGate << std::endl;
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
