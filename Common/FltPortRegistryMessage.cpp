#include "FltPortRegistryMessage.hpp"

KmUmShared::RegistryTemplate1StringMessage::RegistryTemplate1StringMessage(
    MessageCode MessageCode,
    Cpp::String& String1,
    long Status
) : messageCode{ MessageCode },
    string1{ String1 },
    status{Status}
{
}

Cpp::Stream & KmUmShared::RegistryTemplate1StringMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> string1 >> status;
    return Stream;
}

Cpp::Stream & 
KmUmShared::RegistryTemplate1StringMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << FilterMessageHeader{ messageCode } << string1 << status;
    return Stream;
}

KmUmShared::RegistryTemplate2StringMessage::RegistryTemplate2StringMessage(
    MessageCode MessageCode,
    Cpp::String& String1,
    Cpp::String& String2,
    long Status
) : messageCode{MessageCode},
    string1{String1},
    string2{String2},
    status{Status}
{
}

Cpp::Stream & KmUmShared::RegistryTemplate2StringMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> string1 >> string2 >> status;
    return Stream;
}

Cpp::Stream & KmUmShared::RegistryTemplate2StringMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << FilterMessageHeader{ messageCode } << string1<< string2 << status;
    return Stream;
}

KmUmShared::RegistryCreateMessage::RegistryCreateMessage(
    Cpp::String& String1,
    long Status
) : RegistryTemplate1StringMessage{MessageCode::msgRegistryCreate, String1, Status}
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

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistryDeleteKeyMessage & RegistryDeleteKeyMessage)
{
    return RegistryDeleteKeyMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistryDeleteKeyMessage & RegistryDeleteKeyMessage)
{
    return RegistryDeleteKeyMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistryDeleteKeyValueMessage & RegistryDeleteKeyValueMessage)
{
    return RegistryDeleteKeyValueMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistryDeleteKeyValueMessage & RegistryDeleteKeyValueMessage)
{
    return RegistryDeleteKeyValueMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistryLoadKeyMessage & RegistryLoadKeyMessage)
{
    return RegistryLoadKeyMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistryLoadKeyMessage & RegistryLoadKeyMessage)
{
    return RegistryLoadKeyMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const RegistryRenameKeyMessage & RegistryRenameKeyMessage)
{
    return RegistryRenameKeyMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, RegistryRenameKeyMessage & RegistryRenameKeyMessage)
{
    return RegistryRenameKeyMessage.Deserialize(Stream);
}

KmUmShared::RegistrySetValueMessage::RegistrySetValueMessage(
    Cpp::String& String1,
    Cpp::String& String2,
    Cpp::String& String3,
    unsigned long DataType,
    long Status
) : RegistryTemplate3StringMessage { MessageCode::msgRegistrySetValue, String1, String2, String3, DataType, Status}
{
}

KmUmShared::RegistryDeleteKeyValueMessage::RegistryDeleteKeyValueMessage(
    Cpp::String& String1,
    Cpp::String& String2,
    long Status
) : RegistryTemplate2StringMessage{ MessageCode::msgRegistryDeleteValue, String1, String2, Status }
{
}

KmUmShared::RegistryDeleteKeyMessage::RegistryDeleteKeyMessage(
    Cpp::String& String1,
    long Status
) : RegistryTemplate1StringMessage{ MessageCode::msgRegistryDeleteKey, String1, Status }
{
}

KmUmShared::RegistryLoadKeyMessage::RegistryLoadKeyMessage(
    Cpp::String& String1,
    long Status
) : RegistryTemplate1StringMessage{ MessageCode::msgRegistryLoadKey, String1, Status }
{

}

KmUmShared::RegistryRenameKeyMessage::RegistryRenameKeyMessage(
    Cpp::String& String1,
    Cpp::String& String2,
    long Status
) : RegistryTemplate2StringMessage{ MessageCode::msgRegistryRenameKey, String1, String2, Status }
{
}

KmUmShared::RegistryTemplate3StringMessage::RegistryTemplate3StringMessage(
    MessageCode MessageCode, 
    Cpp::String & String1, 
    Cpp::String & String2, 
    Cpp::String & String3, 
    unsigned long DataType,
    long Status
) : messageCode{MessageCode},
    string1{String1},
    string2{String2},
    string3{String3},
    status{Status},
    dataType{DataType}
{
}

Cpp::Stream & KmUmShared::RegistryTemplate3StringMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << FilterMessageHeader{ messageCode } << string1 << string2 << string3 << status << dataType;
    return Stream;
}

Cpp::Stream & KmUmShared::RegistryTemplate3StringMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> string1 >> string2 >> string3 >> status >> dataType;
    return Stream;
}
