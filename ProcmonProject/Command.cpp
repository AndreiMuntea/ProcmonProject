#include "Command.hpp"

Command::Command(
    _In_ const std::string& Description, 
    _In_ std::function<void()> Handler
) : description{Description},
    handler{Handler}
{}

void Command::Execute()
{
    handler();
}

const std::string& 
Command::GetDescription() const
{
    return description;
}
