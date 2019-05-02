#include "FltPortProcessMessage.hpp"
#include "FltPortMessage.hpp"

KmUmShared::ProcessCreateMessage::ProcessCreateMessage(
    unsigned __int64 ParentId,
    Cpp::String& CommandLine
) : commandLine{CommandLine},
    parentId{ParentId}
{
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ProcessCreateMessage & ProcessCreateMessage)
{
    Stream << FilterMessageHeader{ MessageCode::msgProcessCreate } << ProcessCreateMessage.commandLine << ProcessCreateMessage.parentId;
    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, ProcessCreateMessage & ProcessCreateMessage)
{
    Stream >> ProcessCreateMessage.commandLine >> ProcessCreateMessage.parentId;
    return Stream;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ProcessTerminateMessage & ProcessTerminateMessage)
{
    ProcessTerminateMessage;

    Stream << FilterMessageHeader{ MessageCode::msgProcessTerminate };
    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, ProcessTerminateMessage & ProcessTerminateMessage)
{
    ProcessTerminateMessage;
    return Stream;
}
