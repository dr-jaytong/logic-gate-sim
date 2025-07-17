#include <filesystem>
#include "FileHandler.hpp"

#define CPP_MODULE "FILE"

#include "Logging.hpp"

bool FileHandler::FileExists(std::string const &sFileName) 
{
    return std::filesystem::exists(sFileName);
}

FileHandler::FileHandler(std::string const &sFileName)
{
    if (!FileExists(sFileName))
        LOG_ERROR(sFileName + " does not exist!");

    LOG("Opening file: " << sFileName);
    m_File.open(sFileName);
}

FileHandler::~FileHandler()
{
    if (m_File.is_open()) 
        m_File.close(); 
}

std::string FileHandler::GetNextLine()
{
    std::string sFileLine;
    std::getline(m_File, sFileLine);
    return sFileLine;
}
