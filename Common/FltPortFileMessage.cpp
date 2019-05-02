#include "FltPortFileMessage.hpp"

KmUmShared::FileMessage::FileMessage(
    MessageCode MessageCode,
    Cpp::String& String1,
    long Status
) : messageCode{MessageCode},
    string1{String1},
    status{Status}
{
}

Cpp::Stream & KmUmShared::FileMessage::Serialize(Cpp::Stream & Stream) const
{
    Stream << FilterMessageHeader{ messageCode } << string1 << status;
    return Stream;
}

Cpp::Stream & KmUmShared::FileMessage::Deserialize(Cpp::Stream & Stream)
{
    Stream >> string1 >> status;
    return Stream;
}

KmUmShared::FileCreateMessage::FileCreateMessage(
    Cpp::String& String1,
    long Status
) : FileMessage(MessageCode::msgFileCreate, String1, Status)
{
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FileCreateMessage & FileCreateMessage)
{
    return FileCreateMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator>>(Cpp::Stream & Stream, FileCreateMessage & FileCreateMessage)
{
    return FileCreateMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FileCloseMessage & FileCloseMessage)
{
    return FileCloseMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, FileCloseMessage & FileCloseMessage)
{
    return FileCloseMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FileCleanupMessage & FileCleanupMessage)
{
    return FileCleanupMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, FileCleanupMessage & FileCleanupMessage)
{
    return FileCleanupMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FileReadMessage & FileReadMessage)
{
    return FileReadMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, FileReadMessage & FileReadMessage)
{
    return FileReadMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FileWriteMessage & FileWriteMessage)
{
    return FileWriteMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, FileWriteMessage & FileWriteMessage)
{
    return FileWriteMessage.Deserialize(Stream);
}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const FileSetInformationMessage & FileWriteMessage)
{
    return FileWriteMessage.Serialize(Stream);
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, FileSetInformationMessage & FileSetInformationMessage)
{
    return FileSetInformationMessage.Deserialize(Stream);
}

KmUmShared::FileCloseMessage::FileCloseMessage(
    Cpp::String& String1,
    long Status
) : FileMessage(MessageCode::msgFileClose, String1, Status) 
{
}

KmUmShared::FileCleanupMessage::FileCleanupMessage(
    Cpp::String& String1,
    long Status
) : FileMessage(MessageCode::msgFileCleanup, String1, Status) 
{
}

KmUmShared::FileReadMessage::FileReadMessage(
    Cpp::String& String1,
    long Status
) : FileMessage(MessageCode::msgFileRead, String1, Status) 
{
}

KmUmShared::FileWriteMessage::FileWriteMessage(
    Cpp::String& String1,
    long Status
) : FileMessage(MessageCode::msgFileWrite, String1, Status) 
{
}

KmUmShared::FileSetInformationMessage::FileSetInformationMessage(
    Cpp::String& String1,
    long Status
) : FileMessage(MessageCode::msgFileSetInformation, String1, Status)
{
}
