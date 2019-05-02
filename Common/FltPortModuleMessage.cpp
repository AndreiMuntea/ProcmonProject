#include "FltPortModuleMessage.hpp"

KmUmShared::ModuleMessage::ModuleMessage(
    unsigned __int64 ImageBase,
    unsigned __int64 ImageSize,
    const Cpp::String& ImageName
) : imageBase{ImageBase},
    imageSize{ImageSize},
    imageName{ ImageName }
{

}

Cpp::Stream & KmUmShared::operator<<(Cpp::Stream & Stream, const ModuleMessage & ModuleMessage)
{
    Stream << KmUmShared::FilterMessageHeader{ MessageCode::msgModuleLoaded }
        << ModuleMessage.imageName
        << ModuleMessage.imageBase
        << ModuleMessage.imageSize;

    return Stream;
}

Cpp::Stream & KmUmShared::operator >> (Cpp::Stream & Stream, ModuleMessage & ModuleMessage)
{
    Stream >> ModuleMessage.imageName
        >> ModuleMessage.imageBase
        >> ModuleMessage.imageSize;

    return Stream;
}
