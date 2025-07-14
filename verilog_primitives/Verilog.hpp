#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Verilog {
public: 
    struct Wire {
        std::string m_sWireName;
        std::string m_sOutputGate; // The output gate that drives the value of the wire (should only be one)
        std::vector<std::string> m_vInputGates; // The input gates that uses this wire as an input source (could be many)

        explicit Wire(std::string const &sWireName) 
            : m_sWireName(sWireName)
            , m_sOutputGate()
            , m_vInputGates() {}

       ~Wire(){}

        Wire(Wire const &RHS) 
            : m_sWireName(RHS.m_sWireName)
            , m_sOutputGate(RHS.m_sOutputGate)
            , m_vInputGates(RHS.m_vInputGates){}
       
        Wire &operator=(Wire const &RHS) = delete;

        Wire(Wire const &&RHS)
            : m_sWireName(RHS.m_sWireName)
            , m_sOutputGate(RHS.m_sOutputGate)
            , m_vInputGates(std::move(RHS.m_vInputGates)){}

    };


    struct Gate {
        std::string m_sGateType;
        std::string m_sGateIdentifier;
        std::string m_sOutputPortName;
        std::vector<std::string> m_vInputPortNames;
        std::vector<std::string> m_vOutgoingGates; // outgoing nodes

        Gate(std::string const &sGateType, std::string const &sGateIdentifier, std::string const sOutputPortName, std::vector<std::string> const &vInputPortNames) 
            : m_sGateType(sGateType)
            , m_sGateIdentifier(sGateIdentifier)
            , m_sOutputPortName(sOutputPortName)
            , m_vInputPortNames(std::move(vInputPortNames))
            , m_vOutgoingGates() {}

       ~Gate() {}

        Gate          (Gate const &RHS) = delete;
        Gate operator=(Gate const &RHS) = delete;

        Gate(Gate const &&RHS)
            : m_sGateType      (RHS.m_sGateType)
            , m_sGateIdentifier(RHS.m_sGateIdentifier)
            , m_sOutputPortName(RHS.m_sOutputPortName)
            , m_vInputPortNames(std::move(RHS.m_vInputPortNames))
            , m_vOutgoingGates (std::move(RHS.m_vOutgoingGates)) {}
    };

private:
    std::unordered_map<std::string, Gate> m_umGateID2Gates;
    std::unordered_set<std::string> m_usInputPorts; 
    std::unordered_map<std::string, Wire> m_usWireName2GateOutput;
    std::unordered_set<std::string> m_usOutputPorts;

    struct WireHash {
        size_t operator()(Wire const &Wire) const { return std::hash<std::string>()(Wire.m_sWireName); }
    };

public:

    explicit Verilog() : m_umGateID2Gates(), m_usInputPorts(), m_usWireName2GateOutput(), m_usOutputPorts() {}
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    void AddGate (Gate const &input);
    Gate &GetGate(std::string const &sGateIdentifier);

    void AddInputPorts (std::unordered_set<std::string> const &vPorts)       { m_usInputPorts  = std::move(vPorts); } 
    void AddOutputPorts(std::unordered_set<std::string> const &vPorts)       { m_usOutputPorts = std::move(vPorts); }
    void AddWires      (std::unordered_map<std::string, Wire> const &vPorts) { m_usWireName2GateOutput = std::move(vPorts); }

    void Print();
};

#endif
