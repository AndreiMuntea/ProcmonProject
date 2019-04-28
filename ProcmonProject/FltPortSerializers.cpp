#include "FltPortSerializers.hpp"

std::wostream& operator<<(std::wostream& Stream, Cpp::String& String)
{
    Stream.write((const wchar_t*)String.GetNakedPointer(), String.GetSize() / sizeof(wchar_t));
    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessCreateMessage& ProcessCreateMessage)
{
    Stream << "[ProcessCreateMessage]" << std::endl
        << "\t> [Timestamp] " << ProcessCreateMessage.timestamp << std::endl
        << "\t> [Parent ID] " << ProcessCreateMessage.parentPid << std::endl
        << "\t> [Process ID] " << ProcessCreateMessage.processId << std::endl
        << "\t> [Image Path] " << ProcessCreateMessage.imagePath << std::endl
        << "\t> [Command Line] " << ProcessCreateMessage.commandLine << std::endl
        << std::endl;

    return Stream;
}

std::wostream& operator<<(std::wostream& Stream, KmUmShared::ProcessTerminateMessage& ProcessTerminateMessage)
{
    Stream << "[ProcessTerminateMessage]" << std::endl
        << "\t> [Timestamp] " << ProcessTerminateMessage.timestamp << std::endl
        << "\t> [Process ID] " << ProcessTerminateMessage.processId << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ThreadCreateMessage & ThreadCreateMessage)
{
    Stream << "[ThreadCreateMessage]" << std::endl
        << "\t> [Timestamp] " << ThreadCreateMessage.timestamp << std::endl
        << "\t> [Process ID] " << ThreadCreateMessage.processId << std::endl
        << "\t> [Thread ID] " << ThreadCreateMessage.threadId << std::endl
        << std::endl;

    return Stream;
}

std::wostream & operator<<(std::wostream & Stream, KmUmShared::ThreadTerminateMessage & ThreadTerminateMessage)
{
    Stream << "[ThreadTerminateMessage]" << std::endl
        << "\t> [Timestamp] " << ThreadTerminateMessage.timestamp << std::endl
        << "\t> [Process ID] " << ThreadTerminateMessage.processId << std::endl
        << "\t> [Thread ID] " << ThreadTerminateMessage.threadId << std::endl
        << std::endl;

    return Stream;
}
