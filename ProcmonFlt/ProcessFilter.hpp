#ifndef __PROCESS_FILTER_HPP__ 
#define __PROCESS_FILTER_HPP__ 

#include "cpp_allocator_object.hpp"

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

    protected:
        EX_RUNDOWN_REF rundownProtect = { 0 };

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
    };
}

#endif //__PROCESS_FILTER_HPP__ 