#include "CommandLineInterface.hpp"
#include "ParseVerilogFile.hpp"

int main(int argc, char *argv[])
{
    CommandLineInterface CLI;

    CLI.RegisterArgument("--filename", "STRING", true);
    CLI.Parse(argc, argv);

    FileHandler VerilogFile(CLI.GetValue("--filename"));
    Verilog VerilogModule;
    ParseVerilogFile::ParseFile(VerilogModule, VerilogFile);

    VerilogModule.Print();

    ///ParseVerilogFile verilog(CLI.GetValue("--filename"));
    ///verilog.ParseFile(VerilogModule);
}
