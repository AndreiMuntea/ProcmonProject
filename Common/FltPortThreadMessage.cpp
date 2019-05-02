#include "FltPortThreadMessage.hpp"

KmUmShared::ThreadCreateMessage::ThreadCreateMessage(unsigned __int32 ThreadId) : threadId{ThreadId}
{
}


KmUmShared::ThreadTerminateMessage::ThreadTerminateMessage(unsigned __int32 ThreadId) : threadId{ ThreadId }
{
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ThreadCreateMessage & ThreadMessage)
{
    Stream << FilterMessageHeader{ MessageCode::msgThreadCreate } << ThreadMessage.threadId;
    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, ThreadCreateMessage & ThreadMessage)
{
    Stream >> ThreadMessage.threadId;
    return Stream;
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ThreadTerminateMessage & ThreadMessage)
{
    Stream << FilterMessageHeader{ MessageCode::msgThreadTerminate } << ThreadMessage.threadId;
    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, ThreadTerminateMessage & ThreadMessage)
{
    Stream >> ThreadMessage.threadId;
    return Stream;
}