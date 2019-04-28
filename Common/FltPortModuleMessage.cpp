#include "FltPortModuleMessage.hpp"

KmUmShared::ModuleMessage::ModuleMessage(
    unsigned __int32 ProcessId,
    unsigned __int64 Timestamp,
    unsigned __int64 ImageBase,
    unsigned __int64 ImageSize,
    const unsigned __int8* ImageName,
    unsigned __int32 ImageNameSize
) : processId{ProcessId},
    timestamp{Timestamp},
    imageBase{ImageBase},
    imageSize{ImageSize},
    imageName{ImageName, ImageNameSize}
{

}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ModuleMessage & ModuleMessage)
{
    Stream << KmUmShared::FilterMessageHeader{ MessageCode::msgModuleLoaded }
        << ModuleMessage.processId
        << ModuleMessage.timestamp
        << ModuleMessage.imageName
        << ModuleMessage.imageBase
        << ModuleMessage.imageSize;

    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, ModuleMessage & ModuleMessage)
{
    Stream >> ModuleMessage.processId
        >> ModuleMessage.timestamp
        >> ModuleMessage.imageName
        >> ModuleMessage.imageBase
        >> ModuleMessage.imageSize;

    return Stream;
}
