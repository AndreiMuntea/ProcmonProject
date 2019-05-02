#include "ProcessCollector.hpp"
#include "trace.hpp"
#include "ProcessCollector.tmh"

#include "cpp_lockguard.hpp"

Minifilter::Process::Process(
    _In_ const unsigned __int64& StartTimestamp,
    _In_ const Cpp::String& ProcessName,
    _In_ HANDLE ProcessId
) : startTimestamp{StartTimestamp},
    processId{ProcessId},
    processName{ProcessName}
{
}

Cpp::String 
Minifilter::Process::GetProcessName() const
{
    return this->processName;
}

HANDLE 
Minifilter::Process::GetPid() const
{
    return this->processId;
}

unsigned __int64 
Minifilter::Process::GetStartTimestamp() const
{
    return this->startTimestamp;
}

unsigned __int64 
Minifilter::Process::GetEndTimestamp() const
{
    return this->endTimestamp;
}

void 
Minifilter::Process::MarkTerminated(
    unsigned __int64 EndTimestamp
)
{
    this->endTimestamp = EndTimestamp;
}


Minifilter::ProcessCollector::ProcessCollector()
{
    Validate();
}

bool
Minifilter::ProcessCollector::GetProcessName(
    _In_ HANDLE Pid,
    _In_ const unsigned __int64& ActionTimestamp,
    _Inout_ Cpp::String & ProcessName
)
{
    Cpp::SharedLockguard guard(&this->lock);

    auto process = FindProcessUnsafe(this->activeProcesses, Pid, ActionTimestamp);
    if (process)
    {
        ProcessName = process->GetProcessName();
        return true;
    }

    process = FindProcessUnsafe(this->inactiveProcesses, Pid, ActionTimestamp);
    if (process)
    {
        ProcessName = process->GetProcessName();
        return true;
    }

    return false;
}

void 
Minifilter::ProcessCollector::AddProcess(
    _In_ const unsigned __int64& StartTimestamp,
    _In_ const Cpp::String& ProcessName,
    _In_ HANDLE ProcessId
)
{
    Cpp::ExclusiveLockguard guard(&this->lock);
    
    auto process = new Process(StartTimestamp, ProcessName, ProcessId);
    if (process)
    {
        activeProcesses.InsertTail(process);
    }
}

void 
Minifilter::ProcessCollector::TerminateProcess(
    _In_ const unsigned __int64& EndTimestamp,
    _In_ HANDLE ProcessId
)
{
    Cpp::ExclusiveLockguard guard(&this->lock);
    auto process = FindProcessUnsafe(activeProcesses, ProcessId, EndTimestamp);

    if (process)
    {
        process->MarkTerminated(EndTimestamp);
        activeProcesses.RemoveEntry(process);

        inactiveProcesses.InsertTail(process);
    }
}

Minifilter::Process* 
Minifilter::ProcessCollector::FindProcessUnsafe(
    _In_ Cpp::LinkedList<Process>& Processes,
    _In_ HANDLE ProcessId,
    _In_ const unsigned __int64& ActionTimestamp
)
{
    for (auto it = Processes.begin(); it != Processes.end(); ++it)
    {
        if (it.GetRawPointer()->GetPid() != ProcessId)
        {
            continue;
        }
        if (it.GetRawPointer()->GetStartTimestamp() > ActionTimestamp)
        {
            continue;
        }
        if (it.GetRawPointer()->GetEndTimestamp() < ActionTimestamp)
        {
            continue;
        }
        return it.GetRawPointer();
    }


    return nullptr;
}
