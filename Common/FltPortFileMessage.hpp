#ifndef __FLT_PORT_FILE_MESSAGE_HPP__
#define __FLT_PORT_FILE_MESSAGE_HPP__

#include <CppString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"

namespace KmUmShared
{
    class FileMessage
    {
    public:        
        FileMessage(
            MessageCode MessageCode,
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileMessage() = default;
        virtual ~FileMessage() = default;

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

    class FileCreateMessage : public FileMessage
    {
    public:
        FileCreateMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileCreateMessage() = default;
        virtual ~FileCreateMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FileCreateMessage& FileCreateMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FileCreateMessage& FileCreateMessage);
    };

    class FileCloseMessage : public FileMessage
    {
    public:
        FileCloseMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileCloseMessage() = default;
        virtual ~FileCloseMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FileCloseMessage& FileCloseMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FileCloseMessage& FileCloseMessage);
    };

    class FileCleanupMessage : public FileMessage
    {
    public:
        FileCleanupMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileCleanupMessage() = default;
        virtual ~FileCleanupMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FileCleanupMessage& FileCleanupMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FileCleanupMessage& FileCleanupMessage);
    };

    class FileReadMessage : public FileMessage
    {
    public:
        FileReadMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileReadMessage() = default;
        virtual ~FileReadMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FileReadMessage& FileReadMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FileReadMessage& FileReadMessage);
    };

    class FileWriteMessage : public FileMessage
    {
    public:
        FileWriteMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileWriteMessage() = default;
        virtual ~FileWriteMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FileWriteMessage& FileWriteMessage);
        friend Cpp::Stream& operator >> (Cpp::Stream& Stream, FileWriteMessage& FileWriteMessage);
    };

    class FileSetInformationMessage : public FileMessage
    {
    public:
        FileSetInformationMessage(
            unsigned __int64 Timestamp,
            unsigned __int32 ProcessId,
            const unsigned __int8* String1Buffer,
            unsigned __int32 String1BufferSize,
            long Status
        );

        FileSetInformationMessage() = default;
        virtual ~FileSetInformationMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const FileSetInformationMessage& FileSetInformationMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, FileSetInformationMessage& FileSetInformationMessage);
    };
}

#endif // __FLT_PORT_FILE_MESSAGE_HPP__