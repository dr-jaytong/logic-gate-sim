#include <iostream>
#include <filesystem>
#include <stdexcept>
#include "FileHandler.hpp"

bool FileHandler::FileExists(std::string const &sFileName) 
{
    return std::filesystem::exists(sFileName);
}

FileHandler::FileHandler(std::string const &sFileName)
{
    if (!FileExists(sFileName))
        throw std::invalid_argument(sFileName + " does not exist!");

    std::cout << "Opening file: " << sFileName << std::endl;
    m_File.open(sFileName);
    m_sLastLineParsed = "";
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
    m_sLastLineParsed = sFileLine; // Storing last line that was parsed
                                   // because getline will move the file pointer
    return sFileLine;
}
