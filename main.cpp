#include "CommandLineInterface.hpp"
#include "VerilogUtilities.hpp"

#define CPP_MODULE "MAIN"

#include "Logging.hpp"

int main(int argc, char *argv[])
{
    CommandLineInterface CLI;

    CLI.RegisterArgument("--filename", "STRING", true);
    CLI.Parse(argc, argv);

    FileHandler VerilogFile(CLI.GetValue("--filename"));
    Verilog VerilogModule;
    VerilogUtility::ParseFile(VerilogModule, VerilogFile);

    VerilogModule.Levelize();
    VerilogModule.Print();

    ///ParseVerilogFile verilog(CLI.GetValue("--filename"));
    ///verilog.ParseFile(VerilogModule);
    LOG("Program completed");
}
