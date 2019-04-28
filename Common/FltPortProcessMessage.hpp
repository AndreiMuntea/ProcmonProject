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
            _In_ unsigned __int64 Timestamp,
            _In_ unsigned __int32 ParentPid,
            _In_ unsigned __int32 ProcessId,
            _In_opt_ const unsigned __int8* ImagePath, 
            _In_ unsigned __int32 ImagePathSize,
            _In_opt_ const unsigned __int8* CommandLine,
            _In_ unsigned __int32 CommandLineSize
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ProcessCreateMessage& ProcessCreateMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ProcessCreateMessage& ProcessCreateMessage);

    public:
        unsigned __int64 timestamp = 0;
        unsigned __int32 processId = 0;
        unsigned __int32 parentPid = 0;
        Cpp::String imagePath;
        Cpp::String commandLine;
    };

    class ProcessTerminateMessage
    {
    public:
        ProcessTerminateMessage() = default;
        virtual ~ProcessTerminateMessage() = default;

        ProcessTerminateMessage(
            _In_ unsigned __int64 Timestamp,
            _In_ unsigned __int32 ProcessId
        );

        friend Cpp::Stream& operator<<(Cpp::Stream& Stream, const ProcessTerminateMessage& ProcessCreateMessage);
        friend Cpp::Stream& operator>>(Cpp::Stream& Stream, ProcessTerminateMessage& ProcessCreateMessage);

    public:
        unsigned __int64 timestamp = 0;
        unsigned __int32 processId = 0;
    };
}

#endif //__FLT_PORT_PROCESS_CREATE_MESSAGE_HPP__