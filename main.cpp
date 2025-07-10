#include "CommandLineInterface.hpp"
#include "ParseVerilogFile.hpp"

int main(int argc, char *argv[])
{
    CommandLineInterface CLI;

    CLI.RegisterArgument("--filename", "STRING", true);
    CLI.Parse(argc, argv);

    ParseVerilogFile verilog;
    verilog.ParseFile(CLI.GetValue("--filename"));
}
