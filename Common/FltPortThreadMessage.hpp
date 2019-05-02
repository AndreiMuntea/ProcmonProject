#ifndef __FLT_PORT_THREAD_MESSAGE_HPP__
#define __FLT_PORT_THREAD_MESSAGE_HPP__

#include <CppString.hpp>
#include <sal.h>

#include "FltPortMessage.hpp"

namespace KmUmShared
{
    class ThreadCreateMessage
    {
    public:
        ThreadCreateMessage() = default;

        ThreadCreateMessage(
            unsigned __int32 ThreadId
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ThreadCreateMessage& ThreadMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ThreadCreateMessage& ThreadMessage);

    public:
        unsigned __int32 threadId;
    };

    class ThreadTerminateMessage
    {
    public:
        ThreadTerminateMessage() = default;

        ThreadTerminateMessage(
            unsigned __int32 ThreadId
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ThreadTerminateMessage& ThreadMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ThreadTerminateMessage& ThreadMessage);

    public:
        unsigned __int32 threadId;
    };
};

#endif //__FLT_PORT_THREAD_MESSAGE_HPP__