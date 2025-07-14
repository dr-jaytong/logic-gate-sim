#include "Verilog.hpp"

#include <iostream>
#include <stdexcept>
#include <cassert>

void Verilog::AddGate(Verilog::Gate const &inputGate) 
{
    std::unordered_map<std::string, Verilog::Wire>::iterator itFindWire;
    for (auto const &sInputPort : inputGate.m_vInputPortNames) {
        itFindWire = m_usWireName2GateOutput.find(sInputPort);
        if (itFindWire != m_usWireName2GateOutput.end()) 
            itFindWire->second.m_vInputGates.push_back(inputGate.m_sGateIdentifier);
     
        //TODO : Map Primary Inputs to the gates that rely on them?
        ///std::unordered_set<std::string>::iterator itFindPrimaryInput(m_usInputPorts.find(sInputPort));
        ///if (itFindPrimaryInput != m_usInputPorts.end())
        ///    std::cout << "Gate Inputport " << sInputPort << " is a primary input object" << std::endl;

    }
    
    itFindWire = m_usWireName2GateOutput.find(inputGate.m_sOutputPortName);
    if (itFindWire != m_usWireName2GateOutput.end()) {
        assert(itFindWire->second.m_sOutputGate.empty());
        itFindWire->second.m_sOutputGate = inputGate.m_sOutputPortName;
    }

    if (!m_umGateID2Gates.insert({inputGate.m_sGateIdentifier, std::move(inputGate)}).second) 
        throw std::runtime_error(inputGate.m_sGateIdentifier + " already exists !");
}

void Verilog::Print() 
{
    std::cout << "Stored Primary Inputs " << std::endl;
    for (auto const &PI : m_usInputPorts)
        std::cout << PI << std::endl;

    std::cout << "Stored Primary Outputs " << std::endl;
    for (auto const &PI : m_usOutputPorts)
        std::cout << PI << std::endl;

    std::cout << "Stored Wires " << std::endl;
    for (auto const &PI : m_usWireName2GateOutput) {
        std::cout << "   Wire name: " << PI.first << std::endl;
        std::cout << "   \t Gate that drives this wire: " << PI.second.m_sOutputGate << std::endl;
        for (auto const &inputGate : PI.second.m_vInputGates)
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
