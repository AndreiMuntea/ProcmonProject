#ifndef __FLT_PORT_PROCESS_CREATE_MESSAGE_HPP__ 
#define __FLT_PORT_PROCESS_CREATE_MESSAGE_HPP__ 

#include <CppString.hpp>
#include <sal.h>

namespace KmUmShared
{
    class ProcessCreateMessage
    {
    public:
        ProcessCreateMessage() = default;
        virtual ~ProcessCreateMessage() = default;

        ProcessCreateMessage(
            unsigned __int64 ParentId,
            Cpp::String& CommandLine
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ProcessCreateMessage& ProcessCreateMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ProcessCreateMessage& ProcessCreateMessage);

    public:
        unsigned __int64 parentId = 0;
        Cpp::String commandLine;
    };

    class ProcessTerminateMessage
    {
    public:
        ProcessTerminateMessage() = default;
        virtual ~ProcessTerminateMessage() = default;

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ProcessTerminateMessage& ProcessTerminateMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ProcessTerminateMessage& ProcessTerminateMessage);
    };
}

#endif //__FLT_PORT_PROCESS_CREATE_MESSAGE_HPP__