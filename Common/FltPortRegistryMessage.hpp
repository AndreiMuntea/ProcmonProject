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
            Cpp::String& String1,
            long Status
        );

        virtual ~RegistryTemplate1StringMessage() = default;

        Cpp::Stream& Serialize(Cpp::Stream& Stream) const;
        Cpp::Stream& Deserialize(Cpp::Stream& Stream);

    public:
        Cpp::String string1;
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
            Cpp::String& String1,
            Cpp::String& String2,
            long Status
        );

        virtual ~RegistryTemplate2StringMessage() = default;

        Cpp::Stream& Serialize(Cpp::Stream& Stream) const;
        Cpp::Stream& Deserialize(Cpp::Stream& Stream);

    public:
        Cpp::String string1;
        Cpp::String string2;
        long status = 0;
    private:
        MessageCode messageCode;
    };

    class RegistryCreateMessage : public RegistryTemplate1StringMessage
    {
    public:
        RegistryCreateMessage() = default;
        RegistryCreateMessage(
            Cpp::String& String1,
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
            Cpp::String& String1,
            Cpp::String& String2,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistrySetValueMessage& RegistrySetValueMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistrySetValueMessage& RegistrySetValueMessage);
    };

    class RegistryDeleteKeyMessage : public RegistryTemplate1StringMessage
    {
    public:
        RegistryDeleteKeyMessage() = default;
        RegistryDeleteKeyMessage(
            Cpp::String& String1,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistryDeleteKeyMessage& RegistryDeleteKeyMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistryDeleteKeyMessage& RegistryDeleteKeyMessage);
    };

    class RegistryDeleteKeyValueMessage : public RegistryTemplate2StringMessage
    {
    public:
        RegistryDeleteKeyValueMessage() = default;
        RegistryDeleteKeyValueMessage(
            Cpp::String& String1,
            Cpp::String& String2,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistryDeleteKeyValueMessage& RegistryDeleteKeyValueMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistryDeleteKeyValueMessage& RegistryDeleteKeyValueMessage);
    };

    class RegistryLoadKeyMessage : public RegistryTemplate1StringMessage
    {
    public:
        RegistryLoadKeyMessage() = default;
        RegistryLoadKeyMessage(
            Cpp::String& String1,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistryLoadKeyMessage& RegistryLoadKeyMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistryLoadKeyMessage& RegistryLoadKeyMessage);
    };

    class RegistryRenameKeyMessage : public RegistryTemplate2StringMessage
    {
    public:
        RegistryRenameKeyMessage() = default;
        RegistryRenameKeyMessage(
            Cpp::String& String1,
            Cpp::String& String2,
            long Status
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const RegistryRenameKeyMessage& RegistryRenameKeyMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, RegistryRenameKeyMessage& RegistryRenameKeyMessage);
    };
}

#endif // __FLT_PORT_REGISTRY_MESSAGE_HPP__