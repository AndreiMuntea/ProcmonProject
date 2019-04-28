#ifndef __FLT_PORT_THREAD_MESSAGE_HPP__
#define __FLT_PORT_THREAD_MESSAGE_HPP__

#include <CppString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"

namespace KmUmShared
{
    class ThreadMessage
    {
    public:
        ThreadMessage() = default;
        ThreadMessage(
            MessageCode MessageCode,
            unsigned __int32 ProcessId,
            unsigned __int32 ThreadId,
            unsigned __int64 Timestamp
        );

        virtual ~ThreadMessage() = default;

        Cpp::Stream& Serialize(Cpp::Stream& Stream) const;
        Cpp::Stream& Deserialize(Cpp::Stream& Stream);

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ThreadMessage& ThreadMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ThreadMessage& ThreadMessage);

    public:
        unsigned __int32 processId = 0;
        unsigned __int32 threadId = 0;
        unsigned __int64 timestamp = 0;
        
    private:
        MessageCode messageCode = MessageCode::msgMaxValue;
    };

    class ThreadCreateMessage : public ThreadMessage
    {
    public:
        ThreadCreateMessage() = default;

        ThreadCreateMessage(
            unsigned __int32 ProcessId,
            unsigned __int32 ThreadId,
            unsigned __int64 Timestamp
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ThreadCreateMessage& ThreadMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ThreadCreateMessage& ThreadMessage);
    };

    class ThreadTerminateMessage : public ThreadMessage
    {
    public:
        ThreadTerminateMessage() = default;

        ThreadTerminateMessage(
            unsigned __int32 ProcessId,
            unsigned __int32 ThreadId,
            unsigned __int64 Timestamp
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ThreadTerminateMessage& ThreadMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ThreadTerminateMessage& ThreadMessage);
    };
};

#endif //__FLT_PORT_THREAD_MESSAGE_HPP__