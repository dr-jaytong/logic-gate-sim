#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "FileHandler.hpp"

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

        static std::string GetConnectionType(Connection const &RHS) {
            return RHS.m_eType == ConnectionType::PRIMARY_INPUT  ? "pi  "  :
                   RHS.m_eType == ConnectionType::PRIMARY_OUTPUT ? "po  " : "wire"; 
        }

        friend std::ostream &operator<<(std::ostream &os, Connection const &RHS) {
            os << "[ID: \'" << RHS.m_sName << "\', ConnectionType: " << GetConnectionType(RHS) << ", level: " << RHS.m_iLevelNumber << ", driving gate: " << RHS.m_sIncomingGate << " depending gates: ";
            for (auto const &inputGate : RHS.m_vOutgoingGates)
                os << inputGate << ", ";
            os << "]";
            return os;
        }
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

        Gate() = delete;
       ~Gate() {}

        Gate          (Gate const &RHS) = delete;
        Gate operator=(Gate const &RHS) = delete;

        Gate(Gate const &&RHS)
            : m_sGateType      (RHS.m_sGateType)
            , m_sGateIdentifier(RHS.m_sGateIdentifier)
            , m_sOutputPortName(RHS.m_sOutputPortName)
            , m_vInputPortNames(std::move(RHS.m_vInputPortNames))
            , m_iLevelNumber(-1) {}

        friend std::ostream &operator<<(std::ostream &out, Gate const &RHS) {
            out << "[ID: \'" << RHS.m_sGateIdentifier << "\', Type: " << RHS.m_sGateType << ", level: " << RHS.m_iLevelNumber << ", Output port: " << RHS.m_sOutputPortName << ", input ports: ";
            for (auto const &inputPort : RHS.m_vInputPortNames)
                out << inputPort << ", ";
            out << "]";

            return out;
        }
    };

private:
    std::vector<std::string>                    m_vPrimaryInputs;
    std::unordered_map<std::string, Gate>       m_umGateID2Gates;
    std::unordered_map<std::string, Connection> m_umConnectionID2Connection;

    void AddGate         (Gate const &input);
    void AddDFFPorts     (std::vector<std::string> const &vDFFPorts);
    void AddPrimaryInputs(std::vector<std::string> const &vPrimaryInputs) { m_vPrimaryInputs = std::move(vPrimaryInputs); }
    void AddLogic        (Gate const &input);
    void AddConnections  (std::unordered_map<std::string, Connection> const &umConnections); 

    void ParseFile        (FileHandler &&VerilogFile);
    void ConvertModulePort(std::string const &sPort, ConnectionType const eType);

    void Levelize();

    std::string ParseNextVerilogLine(FileHandler &VerilogFile);
    bool IsGate(std::string const &sLine);
    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);
    std::unordered_map<std::string, Verilog::Connection> ExtractConnections(std::string const &sNamesFromString);
    Verilog::Gate ExtractLogicData(std::string const &sGateDataFromString);
    void BuildModule(std::string const &sFileName);

public:

    explicit Verilog(std::string const &sFileName) 
        : m_vPrimaryInputs()
        , m_umGateID2Gates()
        , m_umConnectionID2Connection() { BuildModule(sFileName); }

    Verilog() = delete;
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    friend std::ostream &operator<<(std::ostream &os, Verilog const &RHS) {
        os << std::endl << "[ Stored Connections ]" << std::endl;
        for (auto const &connection : RHS.m_umConnectionID2Connection)
            os << connection.second << std::endl; 

        os << "[ Stored Gates ]" << std::endl;
        for (auto const &gate : RHS.m_umGateID2Gates)
            os << gate.second << std::endl;

        return os;
    }
    void Print();
};

#endif
