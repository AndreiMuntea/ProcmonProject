#include "FltPortProcessTerminateMessage.hpp"
#include "FltPortMessage.hpp"

KmUmShared::ProcessTerminateMessage::ProcessTerminateMessage(
    _In_ unsigned __int64 Timestamp,
    _In_ unsigned __int32 ProcessId
) : timestamp{Timestamp},
    processId{ProcessId}
{
}

Cpp::Stream & 
KmUmShared::operator<<(
    Cpp::Stream & Stream, 
    const ProcessTerminateMessage & ProcessCreateMessage
)
{
    Stream << FilterMessageHeader{ MessageCode::msgProcessTerminate }
        << ProcessCreateMessage.timestamp
        << ProcessCreateMessage.processId;

    return Stream;
}

Cpp::Stream & 
KmUmShared::operator>>(
    Cpp::Stream & Stream, 
    ProcessTerminateMessage & ProcessCreateMessage
)
{
    Stream >> ProcessCreateMessage.timestamp
        >> ProcessCreateMessage.processId;

    return Stream;
}
