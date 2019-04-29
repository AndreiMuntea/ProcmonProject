#include "FltPortRegistryMessage.hpp"

KmUmShared::RegistryTemplate1StringMessage::RegistryTemplate1StringMessage(
    MessageCode MessageCode, 
    unsigned __int64 Timestamp,
    unsigned __int32 ProcessId,
    const unsigned __int8 * String1Buffer, 
    unsigned __int32 String1BufferSize,
    long Status
) : messageCode{ MessageCode },
    timestamp{Timestamp},
    processId{ProcessId},
    string1{ String1Buffer, String1BufferSize },
    status{Status}
{
}

Cpp::Stream & KmUmShared::RegistryTemplate1StringMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> string1 >> timestamp >> processId >> status;
    return Stream;
}

Cpp::Stream & 
KmUmShared::RegistryTemplate1StringMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << FilterMessageHeader{ messageCode }
        << string1
        << timestamp
        << processId
        << status;

    return Stream;
}

KmUmShared::RegistryTemplate2StringMessage::RegistryTemplate2StringMessage(
    MessageCode MessageCode,
    unsigned __int64 Timestamp,
    unsigned __int32 ProcessId,
    const unsigned __int8* String1Buffer,
    unsigned __int32 String1BufferSize,
    const unsigned __int8* String2Buffer,
    unsigned __int32 String2BufferSize,
    long Status
) : messageCode{MessageCode},
    timestamp{Timestamp},
    processId{ProcessId},
    string1{String1Buffer, String1BufferSize},
    string2{String2Buffer, String2BufferSize},
    status{Status}
{
}

Cpp::Stream & KmUmShared::RegistryTemplate2StringMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> string1 >> string2 >> timestamp >> processId >> status;
    return Stream;
}

Cpp::Stream & KmUmShared::RegistryTemplate2StringMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << FilterMessageHeader{ messageCode }
        << string1
        << string2
        << timestamp
        << processId
        << status;
    return Stream;
}

KmUmShared::RegistryCreateMessage::RegistryCreateMessage(
    unsigned __int64 Timestamp,
    unsigned __int32 ProcessId,
    const unsigned __int8 * KeyNameBuffer, 
    unsigned __int32 KeyNameBufferSize,
    long Status
) : RegistryTemplate1StringMessage{MessageCode::msgRegistryCreate, Timestamp, ProcessId, KeyNameBuffer, KeyNameBufferSize, Status}
{
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistryCreateMessage & RegistryCreateMessage)
{
    return RegistryCreateMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistryCreateMessage & RegistryCreateMessage)
{
    return RegistryCreateMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistrySetValueMessage & RegistrySetValueMessage)
{
    return RegistrySetValueMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistrySetValueMessage & RegistrySetValueMessage)
{
    return RegistrySetValueMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistryDeleteKeyValueMessage & RegistryDeleteKeyValueMessage)
{
    return RegistryDeleteKeyValueMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistryDeleteKeyValueMessage & RegistryDeleteKeyValueMessage)
{
    return RegistryDeleteKeyValueMessage.Deserialize(Stream);
}

KmUmShared::RegistrySetValueMessage::RegistrySetValueMessage(
    unsigned __int64 Timestamp,
    unsigned __int32 ProcessId,
    const unsigned __int8* KeyNameBuffer,
    unsigned __int32 KeyNameBufferSize,
    const unsigned __int8* ValueNameBuffer,
    unsigned __int32 ValueNameBufferSize,
    long Status
) : RegistryTemplate2StringMessage { MessageCode::msgRegistrySetValue, Timestamp, ProcessId, KeyNameBuffer, KeyNameBufferSize, ValueNameBuffer, ValueNameBufferSize, Status}
{
}

KmUmShared::RegistryDeleteKeyValueMessage::RegistryDeleteKeyValueMessage(
    unsigned __int64 Timestamp,
    unsigned __int32 ProcessId,
    const unsigned __int8* KeyNameBuffer,
    unsigned __int32 KeyNameBufferSize,
    const unsigned __int8* ValueNameBuffer,
    unsigned __int32 ValueNameBufferSize,
    long Status
) : RegistryTemplate2StringMessage{ MessageCode::msgRegistryDeleteValue, Timestamp, ProcessId, KeyNameBuffer, KeyNameBufferSize, ValueNameBuffer, ValueNameBufferSize, Status }
{
}