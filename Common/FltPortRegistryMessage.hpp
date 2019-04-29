#ifndef __FLT_PORT_REGISTRY_MESSAGE_HPP__
#define __FLT_PORT_REGISTRY_MESSAGE_HPP__

#include <CppString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"

namespace KmUmShared
{
    class RegistryTemplate1StringMessage
    {
    public:
        RegistryTemplate1StringMessage() = default;
        
        RegistryTemplate1StringMessage(
            MessageCode MessageCode,
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        virtual ~RegistryTemplate1StringMessage() = default;

        Cpp::Stream& Serialize(Cpp::Stream& Stream) const;
        Cpp::Stream& Deserialize(Cpp::Stream& Stream);

    public:
        Cpp::String string1;
        unsigned __int64 timestamp = 0;
        unsigned __int32 processId = 0;
        long status = 0;
    private:
        MessageCode messageCode;
    };

    class RegistryTemplate2StringMessage
    {
    public:
        RegistryTemplate2StringMessage() = default;

        RegistryTemplate2StringMessage(
            MessageCode MessageCode,
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            const unsigned __int8* String2Buffer,
            unsigned __int32 String2BufferSize,
            long Status
        );

        virtual ~RegistryTemplate2StringMessage() = default;

        Cpp::Stream& Serialize(Cpp::Stream& Stream) const;
        Cpp::Stream& Deserialize(Cpp::Stream& Stream);

    public:
        Cpp::String string1;
        Cpp::String string2;
        unsigned __int64 timestamp = 0;
        unsigned __int32 processId = 0;
        long status = 0;
    private:
        MessageCode messageCode;
    };

    class RegistryCreateMessage : public RegistryTemplate1StringMessage
    {
    public:
        RegistryCreateMessage() = default;
        RegistryCreateMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* KeyNameBuffer,
            unsigned __int32 KeyNameBufferSize,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistryCreateMessage& RegistryCreateMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistryCreateMessage& RegistryCreateMessage);
    };

    class RegistrySetValueMessage : public RegistryTemplate2StringMessage
    {
    public:
        RegistrySetValueMessage() = default;
        RegistrySetValueMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* KeyNameBuffer,
            unsigned __int32 KeyNameBufferSize,
            const unsigned __int8* ValueNameBuffer,
            unsigned __int32 ValueNameBufferSize,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistrySetValueMessage& RegistrySetValueMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistrySetValueMessage& RegistrySetValueMessage);
    };
}

#endif // __FLT_PORT_REGISTRY_MESSAGE_HPP__