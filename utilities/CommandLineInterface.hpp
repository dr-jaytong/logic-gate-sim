#ifndef CLI_HPP
#define CLI_HPP

#include <map>
#include <string>

class CommandLineInterface
{
private:
    struct ArgData {
        std::string const m_sArgumentName;
        std::string const m_sArgumentType;
        std::string       m_sValue;
        bool              m_bIsSet;
        bool              m_bIsRequired;
        bool              m_bIsFlag;

        ArgData(std::string const &sArgumentName, std::string const &sArgumentType, bool const IsRequired, bool const IsFlag)
            : m_sArgumentName(sArgumentName)
            , m_sArgumentType(sArgumentType)
            , m_sValue("")
            , m_bIsSet(false) 
            , m_bIsRequired(IsRequired)
            , m_bIsFlag(IsFlag) {}

        ArgData(ArgData const &RHS) 
            : m_sArgumentName(RHS.m_sArgumentName)
            , m_sArgumentType(RHS.m_sArgumentType)
            , m_sValue("")
            , m_bIsSet(false) 
            , m_bIsRequired(RHS.m_bIsRequired)
            , m_bIsFlag(RHS.m_bIsFlag) {}

        ArgData &operator=(ArgData const &RHS) = delete;

       ~ArgData() {}
    };
private:
    std::map<std::string, ArgData> m_mapArguments2ArgData;

    void RegisterArgument(std::string const &sArgumentName, std::string const &sArgumentType, bool const IsRequired, bool const IsFlag);
public:

    void RegisterFlag(std::string const &sArgumentName, bool const IsRequired);
    void RegisterArgument(std::string const &sArgumentName, std::string const &sArgumentType, bool const IsRequired);

    bool IsSet(std::string const &sArgumentName);
    bool Parse(int argc, char *argv[]);
    
    std::string GetValue(std::string const &sArgumentName);
};

#endif
