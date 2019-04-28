#include "FltPortThreadMessage.hpp"

KmUmShared::ThreadTerminateMessage::ThreadTerminateMessage(
    unsigned __int32 ProcessId, 
    unsigned __int32 ThreadId,
    unsigned __int64 Timestamp
) : ThreadMessage{MessageCode::msgThreadTerminate, ProcessId, ThreadId, Timestamp}
{
}

KmUmShared::ThreadCreateMessage::ThreadCreateMessage(
    unsigned __int32 ProcessId, 
    unsigned __int32 ThreadId,
    unsigned __int64 Timestamp
) : ThreadMessage{ MessageCode::msgThreadCreate, ProcessId, ThreadId, Timestamp }
{
}

Cpp::Stream & 
KmUmShared::operator<<(
    Cpp::Stream & Stream,
    const ThreadMessage & ThreadMessage
)
{
    Stream << FilterMessageHeader{ ThreadMessage.messageCode }
        << ThreadMessage.processId
        << ThreadMessage.threadId
        << ThreadMessage.timestamp;

    return Stream;
}

Cpp::Stream & 
KmUmShared::operator >> (
    Cpp::Stream & Stream, 
    ThreadMessage & ThreadMessage
)
{
    Stream >> ThreadMessage.processId
        >> ThreadMessage.threadId
        >> ThreadMessage.timestamp;

    return Stream;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ThreadCreateMessage & ThreadMessage)
{
    return ThreadMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, ThreadCreateMessage & ThreadMessage)
{
    return ThreadMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ThreadTerminateMessage & ThreadMessage)
{
    return ThreadMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, ThreadTerminateMessage & ThreadMessage)
{
    return ThreadMessage.Deserialize(Stream);
}

KmUmShared::ThreadMessage::ThreadMessage(
    MessageCode MessageCode, 
    unsigned __int32 ProcessId, 
    unsigned __int32 ThreadId,
    unsigned __int64 Timestamp
) : messageCode{MessageCode},
    processId{ProcessId},
    threadId{ThreadId},
    timestamp{Timestamp}
{
}

Cpp::Stream & KmUmShared::ThreadMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << *this;
    return Stream;
}

Cpp::Stream & KmUmShared::ThreadMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> *this;
    return Stream;
}
