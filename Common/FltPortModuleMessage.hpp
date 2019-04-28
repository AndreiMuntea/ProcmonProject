#ifndef __FLT_PORT_MODULE_MESSAGE_HPP__
#define __FLT_PORT_MODULE_MESSAGE_HPP__

#include <CppString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"

namespace KmUmShared
{
    class ModuleMessage
    {
    public:
        ModuleMessage() = default;
        ModuleMessage(
            unsigned __int32 ProcessId,
            unsigned __int64 Timestamp,
            unsigned __int64 ImageBase,
            unsigned __int64 ImageSize,
            const unsigned __int8* ImageName,
            unsigned __int32 ImageNameSize
        );

        virtual ~ModuleMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ModuleMessage& ModuleMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ModuleMessage& ModuleMessage);

    public:
        unsigned __int32 processId = 0;
        unsigned __int64 timestamp = 0;
        unsigned __int64 imageBase = 0;
        unsigned __int64 imageSize = 0;
        Cpp::String imageName;
    };
}

#endif //__FLT_PORT_MODULE_MESSAGE_HPP__