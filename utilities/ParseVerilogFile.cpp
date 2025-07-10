#include <iostream>

#include "FileHandler.hpp"
#include "ParseVerilogFile.hpp"

void ParseVerilogFile::ParseFile(std::string const &sFileName)
{
    FileHandler VerilogFile(sFileName);
    while (!VerilogFile.eof()) {
        std::cout << VerilogFile.GetNextLine() << std::endl;
    }
}
