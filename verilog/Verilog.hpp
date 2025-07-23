#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Verilog {
public:
    enum ConnectionType { PRIMARY_INPUT = 0, PRIMARY_OUTPUT = 1, WIRE = 2 }; 

    struct Connection { // Primary Input, Primary Output, or Internal Wire
        std::string              m_sName;
        std::string              m_sIncomingGate; // For the gate that drives the signal of this connection (wire, primary output) 
        std::vector<std::string> m_vOutgoingGates; // For gates that depend on this connection (wire, primary input)

        int            m_iLevelNumber;
        ConnectionType m_eType;

        Connection() = delete;
        Connection &operator=(Connection const &RHS) = delete;

        Connection(std::string const &sName, ConnectionType const eType) 
            : m_sName(sName)
            , m_sIncomingGate("")
            , m_vOutgoingGates()
            , m_iLevelNumber(eType == ConnectionType::PRIMARY_INPUT ? 0 : -1)
            , m_eType(eType) {}

        Connection(Connection const &RHS)
            : m_sName         (RHS.m_sName)
            , m_sIncomingGate (RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates))
            , m_iLevelNumber  (RHS.m_iLevelNumber)
            , m_eType         (RHS.m_eType){}

        Connection(Connection const &&RHS)
            : m_sName         (RHS.m_sName)
            , m_sIncomingGate (RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates))
            , m_iLevelNumber  (RHS.m_iLevelNumber)
            , m_eType         (RHS.m_eType) {}
    };

    std::string GetConnectionType(Connection const &RHS) {
        return RHS.m_eType == ConnectionType::PRIMARY_INPUT  ? "primary input"  :
               RHS.m_eType == ConnectionType::PRIMARY_OUTPUT ? "primary output" : "wire"; 
    }

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
    std::vector<std::string>                    m_vPrimaryInputs;
    std::unordered_map<std::string, Gate>       m_umGateID2Gates;
    std::unordered_map<std::string, Connection> m_umConnectionID2Connection;

    void ConvertModulePort(std::string const &sPort, ConnectionType const eType);
    void AddGate (Gate const &input);
    void AddDFFPorts(std::vector<std::string> const &vDFFPorts);
    void AddPrimaryInputs(std::vector<std::string> const &vPrimaryInputs) { m_vPrimaryInputs = std::move(vPrimaryInputs); }
public:

    explicit Verilog() : m_vPrimaryInputs(), m_umGateID2Gates(), m_umConnectionID2Connection() {}
    
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    void AddLogic      (Gate const &input);
    void AddConnections(std::unordered_map<std::string, Connection> const &umConnections); // { m_umConnectionID2Connection.insert(umConnections.begin(), umConnections.end()); }

    void Levelize();

    void Print();
};

#endif
