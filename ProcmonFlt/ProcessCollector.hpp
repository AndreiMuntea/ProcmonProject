#ifndef __PROCESS_COLLECTOR_HPP__ 
#define __PROCESS_COLLECTOR_HPP__

#include "cpp_allocator_object.hpp"
#include "LinkedList.hpp"
#include "cpp_lock.hpp"
#include <CppString.hpp>
#include <CppSemantics.hpp>

namespace Minifilter
{
    class Process : public Cpp::LinkedListEntry
    {
    public:
        Process() = default;
        Process(
            _In_ const unsigned __int64& StartTimestamp,
            _In_ const Cpp::String& ProcessName,
            _In_ HANDLE processId
        );
        virtual ~Process() = default;

        Cpp::String GetProcessName() const;
        HANDLE GetPid() const;

        unsigned __int64 GetStartTimestamp() const;
        unsigned __int64 GetEndTimestamp() const;

        void MarkTerminated(unsigned __int64 EndTimestamp);

    private:
        HANDLE processId = nullptr;
        unsigned __int64 startTimestamp = 0;
        unsigned __int64 endTimestamp = MAXULONGLONG;
        Cpp::String processName;
    };

    class ProcessCollector : public Cpp::CppNonPagedObject<'TCP#'>
    {
    public:
        ProcessCollector();
        virtual ~ProcessCollector() = default;

        bool
        GetProcessName(
            _In_ HANDLE Pid,
            _In_ const unsigned __int64& ActionTimestamp,
            _Inout_ Cpp::String& ProcessName
        );
        
        void 
        AddProcess(
            _In_ const unsigned __int64& StartTimestamp,
            _In_ const Cpp::String& ProcessName,
            _In_ HANDLE ProcessId
        );

        void 
        TerminateProcess(
            _In_ const unsigned __int64& StartTimestamp,
            _In_ HANDLE ProcessId
        );

    private:
        Process* 
        FindProcessUnsafe(
            _In_ Cpp::LinkedList<Process>& Processes,
            _In_ HANDLE ProcessId,
            _In_ const unsigned __int64& ActionTimestamp
        );

        Cpp::LinkedList<Process> activeProcesses;
        Cpp::LinkedList<Process> inactiveProcesses;
        Cpp::Pushlock lock;
    };
}

#endif //__PROCESS_COLLECTOR_HPP__