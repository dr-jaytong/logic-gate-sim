#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Verilog {
private:    
    struct Gate {
        std::string m_sGateIdentifier;
        std::vector<std::string> m_vPortNames;

        Gate(std::string const &sGateIdentifier, std::vector<std::string> const &vPortNames) : m_sGateIdentifier(sGateIdentifier), m_vPortNames(std::move(vPortNames)) {}
       ~Gate() {}

        void AddPort(std::string const &sPortName);
    };

    struct GateHash {
        size_t operator()(std::string const &sInput) const { return std::hash<std::string>()(sInput); }
    };

    std::unordered_map<std::string, Gate, GateHash> m_umGateID2Gate;

public:

    explicit Verilog() : m_umGateID2Gate() {}
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    void AddGate(std::string const &sGateIdentifier, std::vector<std::string> const &vPorts);
    Gate GetGate(std::string const &sGateIdentifier);


};

#endif
