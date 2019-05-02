#ifndef __COMMAND_INTERPRETER_HPP__ 
#define __COMMAND_INTERPRETER_HPP__ 

#include <string>
#include <unordered_map>
#include <shared_mutex>

#include "Command.hpp"

class CommandInterpreter
{
public:
    CommandInterpreter();
    ~CommandInterpreter() = default;

    void Execute(_In_ const std::string& Command);

private:
    void PrintHelpCommand();
    void ExitCommand();

    void StartThreadPoolCommand();
    void StopThreadPoolCommand();

    void ConnectFltPortCommand();
    void DisconnectFltPortCommand();

    void ProtectProcessCommand();

    void UpdateFeatureCommand(bool Enable);
    void UpdateBlacklistedFolder(bool Blacklist);

    void PrintAvailableFeatures();
    std::unordered_map<std::string, Command> availableCommands;
};

#endif //__COMMAND_INTERPRETER_HPP__