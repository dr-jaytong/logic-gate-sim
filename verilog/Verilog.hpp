#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <ostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <bitset>

#include "FileHandler.hpp"

class Verilog {
private:
    enum ConnectionType { PRIMARY_INPUT = 0, PRIMARY_OUTPUT = 1, WIRE = 2, PRIMARY_OUTPUT_DFF = 3, PRIMARY_INPUT_DFF = 4 }; 

    struct Connection { // Primary Input, Primary Output, or Internal Wire
        std::string              m_sName;
        std::string              m_sIncomingGate; // For the gate that drives the signal of this connection (wire, primary output) 
        std::vector<std::string> m_vOutgoingGates; // For gates that depend on this connection (wire, primary input)

        int m_iLevelNumber;
        int m_iNetAddress;

        ConnectionType m_eType;

        Connection() = delete;
        Connection &operator=(Connection const &RHS) = delete;

        Connection(std::string const &sName, ConnectionType const eType, int const iNetAddress) 
            : m_sName(sName)
            , m_sIncomingGate("")
            , m_vOutgoingGates()
            , m_iLevelNumber(eType == ConnectionType::PRIMARY_INPUT ? 0 : -1)
            , m_iNetAddress(iNetAddress)
            , m_eType(eType) {}

        Connection(Connection const &RHS)
            : m_sName         (RHS.m_sName)
            , m_sIncomingGate (RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates))
            , m_iLevelNumber  (RHS.m_iLevelNumber)
            , m_iNetAddress   (RHS.m_iNetAddress)
            , m_eType         (RHS.m_eType){}

        Connection(Connection const &&RHS)
            : m_sName         (RHS.m_sName)
            , m_sIncomingGate (RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates))
            , m_iLevelNumber  (RHS.m_iLevelNumber)
            , m_iNetAddress   (RHS.m_iNetAddress)
            , m_eType         (RHS.m_eType) {}

        static std::string GetConnectionType(Connection const RHS) {
            return RHS.m_eType == ConnectionType::PRIMARY_INPUT  ?     "pi    " :
                   RHS.m_eType == ConnectionType::PRIMARY_OUTPUT ?     "po    " :
                   RHS.m_eType == ConnectionType::PRIMARY_OUTPUT_DFF ? "po-dff" : 
                   RHS.m_eType == ConnectionType::PRIMARY_INPUT_DFF  ? "pi-dff" : 
                   "wire"; 
        }

        friend std::ostream &operator<<(std::ostream &os, Connection const &RHS) {
            os << "[ID: \'" << RHS.m_sName << "\', ConnectionType: " << GetConnectionType(RHS) << ", NetAddr: " << RHS.m_iNetAddress << " level: " << RHS.m_iLevelNumber << ", driving gate: " << RHS.m_sIncomingGate << ", depending gates: ";
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
        int         m_iLevelNumber;

        std::vector<std::string> m_vInputPortNames;

        Gate(std::string              const &sGateType, 
             std::string              const &sGateIdentifier, 
             std::string              const sOutputPortName, 
             std::vector<std::string> const &vInputPortNames) 
            : m_sGateType      (sGateType)
            , m_sGateIdentifier(sGateIdentifier)
            , m_sOutputPortName(sOutputPortName)
            , m_iLevelNumber   (-1)
            , m_vInputPortNames(std::move(vInputPortNames)) {}

        Gate &operator=(Gate const &RHS) = delete;
        Gate() = delete;
       ~Gate() {}

        friend std::ostream &operator<<(std::ostream &out, Gate const &RHS) {
            out << "[ID: \'" << RHS.m_sGateIdentifier << "\', Type: " << RHS.m_sGateType << ", level: " << RHS.m_iLevelNumber << ", Output port: " << RHS.m_sOutputPortName << ", input ports: ";
            for (auto const &inputPort : RHS.m_vInputPortNames)
                out << inputPort << ", ";
            out << "]";

            return out;
        }
    };

    struct ModuleCircuitData {
        std::vector<std::string>                    vPrimaryInputs;
        std::unordered_map<std::string, Gate>       umGateID2Gates;
        std::unordered_map<std::string, Connection> umConnectionID2Connection;
        std::unordered_map<std::string, int>        umGateType2TotalInstance;

        ModuleCircuitData()
            : vPrimaryInputs()
            , umGateID2Gates()
            , umConnectionID2Connection()
            , umGateType2TotalInstance() {}

        ModuleCircuitData           (ModuleCircuitData const &RHS)  = delete;
        ModuleCircuitData           (ModuleCircuitData const &&RHS) = delete;
        ModuleCircuitData &operator=(ModuleCircuitData const &RHS)  = delete;
    };


    struct EncodedCircuitData {
        std::vector<int>            vNets;
        std::vector<std::bitset<8>> vGates;

        EncodedCircuitData()
            : vNets()
            , vGates() {}

        EncodedCircuitData           (EncodedCircuitData const &RHS)  = delete;
        EncodedCircuitData           (EncodedCircuitData const &&RHS) = delete;
        EncodedCircuitData &operator=(EncodedCircuitData const &RHS)  = delete;
    };

private:
    ModuleCircuitData  m_ModuleData;
    EncodedCircuitData m_EncodedData;

    void AddGate         (Gate const &input);
    void AddDFFPorts     (std::vector<std::string> const &vDFFPorts);
    void AddPrimaryInputs(std::vector<std::string> const &vPrimaryInputs) { m_ModuleData.vPrimaryInputs = std::move(vPrimaryInputs); }
    void AddLogic        (Gate const &input);
    void AddConnections  (std::unordered_map<std::string, Connection> const &umConnections); 

    void ResolveWireFanouts();
    void ParseFile        (FileHandler &&VerilogFile);
    bool ConvertModulePort(std::string const &sPort, ConnectionType const eType);
    
    using Level_2_GateID = std::pair<int, std::string>;

    struct MinHeapComparator{
        bool operator()(Level_2_GateID const &A, Level_2_GateID const &B) const {
            return A.first > B.first;
        }
    };
    using PQLevel2GateID = std::priority_queue<Level_2_GateID, std::vector<Level_2_GateID>, MinHeapComparator>;
    PQLevel2GateID Levelize();

    void GenerateGateData(std::priority_queue<Level_2_GateID, std::vector<Level_2_GateID>, MinHeapComparator> &&in);

    std::string ParseNextVerilogLine(FileHandler &VerilogFile);
    bool IsGate(std::string const &sLine);
    std::bitset<8> const GetGateEncoding(std::string const &sGateType);

    std::vector<std::string> ExtractPortNames(std::string const &sPortsFromString);
    std::unordered_map<std::string, Verilog::Connection> ExtractConnections(std::string const &sNamesFromString, int &iNetAddress);
    Verilog::Gate ExtractLogicData(std::string const &sGateDataFromString);
    void BuildModule(std::string const &sFileName);

public:

    explicit Verilog(std::string const &sFileName) 
        : m_ModuleData()
        , m_EncodedData() { BuildModule(sFileName); }

    Verilog() = delete;
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    friend std::ostream &operator<<(std::ostream &os, Verilog const &RHS) {
        os << std::endl << "[ Stored Connections ]" << std::endl;
        for (auto const &connection : RHS.m_ModuleData.umConnectionID2Connection)
            os << connection.second << std::endl; 

        os << "[ Stored Gates ]" << std::endl;
        for (auto const &gate : RHS.m_ModuleData.umGateID2Gates)
            os << gate.second << std::endl;

        return os;
    }

    void PrintModuleStats();
};

#endif
