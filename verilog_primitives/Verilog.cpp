#include "Verilog.hpp"

#include <stdexcept>

void Verilog::AddGate(std::string const &sGateIdentifier, std::vector<std::string> const &vPorts)
{
    if (!m_umGateID2Gate.emplace(sGateIdentifier, Verilog::Gate(sGateIdentifier, vPorts)).second) 
        throw std::runtime_error(sGateIdentifier + " already exists !");
}
