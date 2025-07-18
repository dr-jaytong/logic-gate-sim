#include "Verilog.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>

#define CPP_MODULE "VERL"

#include "Logging.hpp"

void Verilog::AddGate(Verilog::Gate const &inputGate) 
{
    std::unordered_map<std::string, Verilog::Connection>::iterator itFindWire;
    for (auto const &sInputPort : inputGate.m_vInputPortNames) {
        itFindWire = m_umWireName2GateOutput.find(sInputPort);
        if (itFindWire != m_umWireName2GateOutput.end()) 
            itFindWire->second.m_vOutgoingGates.push_back(inputGate.m_sGateIdentifier);
    
        std::unordered_map<std::string, Verilog::Connection>::iterator itFindPI;
        itFindPI = m_umInputPorts.find(sInputPort);
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
        std::cout << "   Wire name: " << PI.first << std::endl;
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
        std::cout << std::endl << "==== " << std::endl;
    }


}
