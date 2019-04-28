#include "FltPortProcessMessage.hpp"
#include "FltPortMessage.hpp"

KmUmShared::ProcessCreateMessage::ProcessCreateMessage(
    _In_ unsigned __int64 Timestamp,
    _In_ unsigned __int32 ParentPid,
    _In_ unsigned __int32 ProcessId,
    _In_opt_ const unsigned __int8* ImagePath,
    _In_ unsigned __int32 ImagePathSize,
    _In_opt_ const unsigned __int8* CommandLine,
    _In_ unsigned __int32 CommandLineSize
) : timestamp{Timestamp},
    parentPid{ParentPid},
    processId{ProcessId},
    imagePath{ImagePath, ImagePathSize},
    commandLine{CommandLine, CommandLineSize}
{
}

Cpp::Stream &
KmUmShared::operator<<(
    Cpp::Stream & Stream, 
    const ProcessCreateMessage & ProcessCreateMessage
)
{
    Stream << FilterMessageHeader{ MessageCode::msgProcessCreate }
           << ProcessCreateMessage.timestamp
           << ProcessCreateMessage.parentPid
           << ProcessCreateMessage.processId
           << ProcessCreateMessage.imagePath
           << ProcessCreateMessage.commandLine;

    return Stream;
}

Cpp::Stream &
KmUmShared::operator >> (
    Cpp::Stream & Stream, 
    ProcessCreateMessage & ProcessCreateMessage
)
{
    Stream >> ProcessCreateMessage.timestamp
           >> ProcessCreateMessage.parentPid
           >> ProcessCreateMessage.processId
           >> ProcessCreateMessage.imagePath
           >> ProcessCreateMessage.commandLine;
    
    return Stream;
}

KmUmShared::ProcessTerminateMessage::ProcessTerminateMessage(
    _In_ unsigned __int64 Timestamp,
    _In_ unsigned __int32 ProcessId
) : timestamp{ Timestamp },
processId{ ProcessId }
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
KmUmShared::operator >> (
    Cpp::Stream & Stream,
    ProcessTerminateMessage & ProcessCreateMessage
    )
{
    Stream >> ProcessCreateMessage.timestamp
        >> ProcessCreateMessage.processId;

    return Stream;
}
