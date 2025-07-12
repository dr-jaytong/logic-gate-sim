#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Verilog {
private:    
    struct Gate {
        std::string m_sGateIdentifier;
        std::string m_sOutputPortName;
        std::vector<std::string> m_vInputPortNames;

        Gate(std::string const &sGateIdentifier, std::string const sOutputPortName, std::vector<std::string> const &vInputPortNames) 
            : m_sGateIdentifier(sGateIdentifier)
            , m_sOutputPortName(sOutputPortName)
            , m_vInputPortNames(std::move(vInputPortNames)) {}

       ~Gate() {}

    };

    struct GateHash {
        size_t operator()(std::string const &sInput) const { return std::hash<std::string>()(sInput); }
    };

    std::unordered_map<std::string, Gate, GateHash> m_umGateID2Gates;

    std::unordered_set<std::string> m_usInputPorts; // Use unordered set?
    std::unordered_set<std::string> m_usWires;
    std::unordered_set<std::string> m_usOutputPorts;

public:

    explicit Verilog() : m_umGateID2Gates(), m_usInputPorts(), m_usWires(), m_usOutputPorts() {}
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    void AddGate       (std::string const &sGateIdentifier, std::string const &sOutputPortName, std::vector<std::string> const &vPorts);
    Gate &GetGate      (std::string const &sGateIdentifier);

    void AddInputPorts (std::vector<std::string> const &vPorts);
    void AddOutputPorts(std::vector<std::string> const &vPorts);
    void AddWires      (std::vector<std::string> const &vPorts);

};

#endif
