#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Verilog {
public:

    struct Connection { // Primary Input, Primary Output, or Internal Wire
        std::string m_sName;
        std::string m_sIncomingGate; // For the gate that drives the signal of this connection (wire, primary output) 
        std::vector<std::string> m_vOutgoingGates; // For gates that depend on this connection (wire, primary input)

        int m_iLevelNumber;

        Connection &operator=(Connection const &RHS) = delete;

        Connection(std::string const &sName, int const iLevelNumber) // For primary inputs 
            : m_sName(sName)
            , m_sIncomingGate("")
            , m_vOutgoingGates()
            , m_iLevelNumber(iLevelNumber) {}

        Connection(std::string const &sName)
            : Connection(sName, -1) {}

        Connection(Connection const &RHS)
            : m_sName(RHS.m_sName)
            , m_sIncomingGate(RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates))
            , m_iLevelNumber(RHS.m_iLevelNumber) {}

        Connection(Connection const &&RHS)
            : m_sName(RHS.m_sName)
            , m_sIncomingGate(RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates))
            , m_iLevelNumber(RHS.m_iLevelNumber) {}
    };

    struct Gate {
        std::string m_sGateType;
        std::string m_sGateIdentifier;
        std::string m_sOutputPortName;
        std::vector<std::string> m_vInputPortNames;
        int m_iLevelNumber;

        Gate(std::string              const &sGateType, 
             std::string              const &sGateIdentifier, 
             std::string              const sOutputPortName, 
             std::vector<std::string> const &vInputPortNames) 
            : m_sGateType(sGateType)
            , m_sGateIdentifier(sGateIdentifier)
            , m_sOutputPortName(sOutputPortName)
            , m_vInputPortNames(std::move(vInputPortNames))
            , m_iLevelNumber(-1){}

       ~Gate() {}

        Gate          (Gate const &RHS) = delete;
        Gate operator=(Gate const &RHS) = delete;

        Gate(Gate const &&RHS)
            : m_sGateType      (RHS.m_sGateType)
            , m_sGateIdentifier(RHS.m_sGateIdentifier)
            , m_sOutputPortName(RHS.m_sOutputPortName)
            , m_vInputPortNames(std::move(RHS.m_vInputPortNames))
            , m_iLevelNumber(-1) {}
    };

private:
    std::unordered_map<std::string, Gate>       m_umGateID2Gates;
    std::unordered_map<std::string, Connection> m_umInputPorts;
    std::unordered_map<std::string, Connection> m_umWires;
    std::unordered_map<std::string, Connection> m_umOutputPorts;

public:

    explicit Verilog() : m_umGateID2Gates(), m_umInputPorts(), m_umWires(), m_umOutputPorts() {}
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    void AddGate (Gate const &input);
    Gate &GetGate(std::string const &sGateIdentifier);

    void AddInputPorts (std::unordered_map<std::string, Connection> const &umPorts) { m_umInputPorts  = std::move(umPorts); } 
    void AddOutputPorts(std::unordered_map<std::string, Connection> const &umPorts) { m_umOutputPorts = std::move(umPorts); }
    void AddWires      (std::unordered_map<std::string, Connection> const &umPorts) { m_umWires       = std::move(umPorts); }

    void Levelize();

    void Print();
};

#endif
