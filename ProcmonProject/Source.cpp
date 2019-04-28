#include "trace.h"
#include "source.tmh"
#include "CommandInterpreter.hpp"
#include "GlobalData.hpp"


#include <iostream>

int main()
{
    WPP_INIT_TRACING(NULL);

    gGlobalData.CommandInterpreter.Execute("Help");

    std::string command;
    while (gGlobalData.IsApplicationRunning)
    {
        std::cout << "Your command: " << std::endl;
        std::getline(std::cin, command);
        std::cout << "===================" << std::endl;
        gGlobalData.CommandInterpreter.Execute(command);
    }

    WPP_CLEANUP();
    return 0;
}