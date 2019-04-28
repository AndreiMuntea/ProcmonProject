#ifndef __COMMAND_HPP__ 
#define __COMMAND_HPP__

#include <string>
#include <functional>

class Command
{
public:
    Command(
        _In_ const std::string& Description,
        _In_ std::function<void()> Handler
    );
    ~Command() = default;

    void Execute();
    const std::string& GetDescription() const;

private:
    std::string description;
    std::function<void()> handler;
};

#endif //__COMMAND_HPP__