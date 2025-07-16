#ifndef CLASS_VERILOG_HPP
#define CLASS_VERILOG_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Verilog {
public:

    struct PortEntity {
        std::string m_sName;
        std::string m_sIncomingGate; // For the gate that drives the wire or primary output
        std::vector<std::string> m_vOutgoingGates; // For gates that depend on this entity (wire, primary input)

        PortEntity(std::string const &sName) 
            : m_sName(sName)
            , m_sIncomingGate("")
            , m_vOutgoingGates(){}

        PortEntity           (PortEntity const &RHS) = delete;
        PortEntity &operator=(PortEntity const &RHS) = delete;
        
        PortEntity(PortEntity const &&RHS)
            : m_sName(RHS.m_sName)
            , m_sIncomingGate(RHS.m_sIncomingGate)
            , m_vOutgoingGates(std::move(RHS.m_vOutgoingGates)) {}
    };

    struct PrimaryInput : public PortEntity {
        explicit PrimaryInput(std::string const &sName)
            : PortEntity(sName) {}

       ~PrimaryInput(){}

        PrimaryInput &operator=(PrimaryInput const &RHS) = delete;

        PrimaryInput(PrimaryInput const  &RHS) : PortEntity(std::move(RHS)){} 
        PrimaryInput(PrimaryInput const &&RHS) : PortEntity(std::move(RHS)){}
    };

    struct PrimaryOutput : public PortEntity {
        explicit PrimaryOutput(std::string const &sName)
            : PortEntity(sName) {}

       ~PrimaryOutput() {}
        
        PrimaryOutput &operator=(PrimaryOutput const &RHS) = delete;

        PrimaryOutput(PrimaryOutput const  &RHS) : PortEntity(std::move(RHS)){}
        PrimaryOutput(PrimaryOutput const &&RHS) : PortEntity(std::move(RHS)){}
    };

    struct Wire : public PortEntity {
        explicit Wire(std::string const &sName)
            : PortEntity(sName){}

       ~Wire(){}

        Wire &operator=(Wire const &RHS) = delete;

        Wire(Wire const &RHS)  : PortEntity(std::move(RHS)){}
        Wire(Wire const &&RHS) : PortEntity(std::move(RHS)){}
    };

    ///struct Wire {
    ///    std::string m_sWireName;
    ///    std::string m_sOutputGate; // The output gate that drives the value of the wire (should only be one)
    ///    std::vector<std::string> m_vInputGates; // The input gates that uses this wire as an input source (could be many)

    ///    explicit Wire(std::string const &sWireName) 
    ///        : m_sWireName(sWireName)
    ///        , m_sOutputGate()
    ///        , m_vInputGates() {}

    ///   ~Wire(){}

    ///    Wire(Wire const &RHS) 
    ///        : m_sWireName(RHS.m_sWireName)
    ///        , m_sOutputGate(RHS.m_sOutputGate)
    ///        , m_vInputGates(RHS.m_vInputGates){}
    ///   
    ///    Wire &operator=(Wire const &RHS) = delete;

    ///    Wire(Wire const &&RHS)
    ///        : m_sWireName(RHS.m_sWireName)
    ///        , m_sOutputGate(RHS.m_sOutputGate)
    ///        , m_vInputGates(std::move(RHS.m_vInputGates)){}

    ///};

    struct Gate {
        std::string m_sGateType;
        std::string m_sGateIdentifier;
        std::string m_sOutputPortName;
        std::vector<std::string> m_vInputPortNames;
        std::vector<std::string> m_vOutgoingGates; // outgoing nodes

        Gate(std::string              const &sGateType, 
             std::string              const &sGateIdentifier, 
             std::string              const sOutputPortName, 
             std::vector<std::string> const &vInputPortNames) 
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
    ///std::unordered_set<std::string> m_usInputPorts; 
    std::unordered_map<std::string, PrimaryInput> m_umInputPorts;
    std::unordered_map<std::string, Wire> m_umWireName2GateOutput;
    ///std::unordered_set<std::string> m_usOutputPorts;

    std::unordered_map<std::string, PrimaryOutput> m_umOutputPorts;

    struct WireHash {
        size_t operator()(Wire const &Wire) const { return std::hash<std::string>()(Wire.m_sName); }
    };

public:

    explicit Verilog() : m_umGateID2Gates(), m_umInputPorts(), m_umWireName2GateOutput(), m_umOutputPorts() {}
   ~Verilog() {}

    Verilog           (Verilog const &RHS)  = delete; // Disable copy
    Verilog           (Verilog const &&RHS) = delete; // Disable move
    Verilog &operator=(Verilog const &RHS)  = delete; // Disable assign

    void AddGate (Gate const &input);
    Gate &GetGate(std::string const &sGateIdentifier);

    void AddInputPorts (std::unordered_map<std::string, PrimaryInput>  const &umPorts) { m_umInputPorts  = std::move(umPorts); } 
    void AddOutputPorts(std::unordered_map<std::string, PrimaryOutput> const &umPorts) { m_umOutputPorts = std::move(umPorts); }
    void AddWires      (std::unordered_map<std::string, Wire>          const &umPorts) { m_umWireName2GateOutput = std::move(umPorts); }

    void Print();
};

#endif
