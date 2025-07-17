#include "CommandLineInterface.hpp"

#define CPP_MODULE "CLII"

#include "Logging.hpp"

#include <iostream>
#include <stdexcept>

void CommandLineInterface::RegisterFlag(std::string const &sArgumentName, bool const IsRequired)
{
    RegisterArgument(sArgumentName, "", IsRequired, true); 
}

void CommandLineInterface::RegisterArgument(std::string const &sArgumentName, std::string const &sArgumentType, bool const IsRequired)
{
    RegisterArgument(sArgumentName, sArgumentType, IsRequired, false);
}

void CommandLineInterface::RegisterArgument(std::string const &sArgumentName, std::string const &sArgumentType, bool const IsRequired, bool const IsFlag)
{
    std::string const &sArgSubstring(sArgumentName.substr(0, 2));
    if (sArgumentName.find('-') == std::string::npos) 
        LOG_ERROR("Arguments must be prepended with - or a --")

    if (m_mapArguments2ArgData.find(sArgumentName) != m_mapArguments2ArgData.end())
        LOG_ERROR(sArgumentName + " was already registered!");

    m_mapArguments2ArgData.insert({sArgumentName, ArgData(sArgumentName, sArgumentType, IsRequired, IsFlag)});
}

bool CommandLineInterface::Parse(int argc, char *argv[])
{
    if (argc == 0) 
        LOG_ERROR("Insufficient parameters");

    for (int iArgCounter = 1; iArgCounter < argc; ++iArgCounter) {
        std::string const sArgumentName(argv[iArgCounter]);
        std::map<std::string, ArgData>::iterator itFindArg(m_mapArguments2ArgData.find(sArgumentName));
        if (itFindArg == m_mapArguments2ArgData.end())
            LOG_ERROR("Unknown argument " + sArgumentName);

        itFindArg->second.m_bIsSet = true;
        if (!itFindArg->second.m_bIsFlag) {
            ++iArgCounter;
            if (iArgCounter > argc || argv[iArgCounter] == nullptr)
                LOG_ERROR(itFindArg->second.m_sArgumentName + " is missing an operand!");

            std::string const sArgumentValue(argv[iArgCounter]);
            std::string const sArgumentValueSubstring(sArgumentValue.substr(0, 2));
            if (sArgumentValueSubstring.find('-') != std::string::npos)
                LOG_ERROR(itFindArg->second.m_sArgumentName + " requires an operand");
            itFindArg->second.m_sValue = argv[iArgCounter];
        }
    }

    // Sanity check
    
    for (auto const &ArgData : m_mapArguments2ArgData) {
        if (ArgData.second.m_bIsRequired && !ArgData.second.m_bIsSet)
            LOG_ERROR(ArgData.second.m_sArgumentName + " was not set and it is required!");
    }

    return true;
}

std::string CommandLineInterface::GetValue(std::string const &sArgumentName)
{
    std::string sArgValue("");
    try { 
        sArgValue = m_mapArguments2ArgData.at(sArgumentName).m_sValue;
    } catch (std::out_of_range const &ex) {
        std::cerr << "Exception caught: " << ex.what() << " at line(" << __LINE__ << "), Argument \'" << sArgumentName << "\' was not registered" << std::endl;
        exit(EXIT_FAILURE);
    }
    return sArgValue;
}
