#include "Verilog.hpp"

#include <stdexcept>

void Verilog::AddGate(std::string const &sGateIdentifier, std::string const &sOutputPortName, std::vector<std::string> const &vInputPorts)
{
    if (!m_umGateID2Gates.emplace(sGateIdentifier, Verilog::Gate(sGateIdentifier, sOutputPortName, vInputPorts)).second) 
        throw std::runtime_error(sGateIdentifier + " already exists !");
}
