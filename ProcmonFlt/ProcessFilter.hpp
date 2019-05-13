#ifndef __PROCESS_FILTER_HPP__ 
#define __PROCESS_FILTER_HPP__ 

#include "cpp_allocator_object.hpp"
#include "FltPort.hpp"

namespace Minifilter
{
    class ProcessFilter : public Cpp::CppNonPagedObject<'TFP#'>
    {
    public:

        ProcessFilter();
        virtual ~ProcessFilter();

        static void
        ProcessCreateNotifyRoutine(
            _Inout_ PEPROCESS Process,
            _In_ HANDLE ProcessId,
            _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
        );

    private:
        static bool 
        IsActionMonitored(
            _In_ HANDLE ProcessId,
            _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
        );

        static void 
        HandleProcessCreate(
            _Inout_ PEPROCESS Process,
            _In_ HANDLE ProcessId,
            _Inout_ PPS_CREATE_NOTIFY_INFO CreateInfo
        );

        static void 
        HandleProcessTerminate(
            _Inout_ PEPROCESS Process,
            _In_ HANDLE ProcessId
        );

        static NTSTATUS 
        CheckForRemoteShell(
            _Inout_ PEPROCESS Process,
            _In_ HANDLE ProcessId
        );

        static NTSTATUS 
        CheckPebStandardHandles(
            _Inout_ PEPROCESS Process,
            _In_ HANDLE ProcessHandle,
            _In_ PPEB Peb
        );

        static bool
        IsSocketHandle(
            _In_ HANDLE ProcessHandle,
            _In_ HANDLE Handle
        );
    };
}

#endif //__PROCESS_FILTER_HPP__ 