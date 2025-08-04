#include "CommandLineInterface.hpp"
#include "Verilog.hpp"

#define CPP_MODULE "MAIN"

#include "Logging.hpp"

int main(int argc, char *argv[])
{
    CommandLineInterface CLI;

    CLI.RegisterArgument("--filename", "STRING", true);
    CLI.Parse(argc, argv);

    Verilog VerilogModule(CLI.GetValue("--filename"));
    VerilogModule.PrintModuleStats();
    LOG("Program completed");
}
