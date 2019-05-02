#include "ProcessFilter.hpp"
#include "trace.hpp"
#include "ProcessFilter.tmh"

#include "GlobalData.hpp"

#include "../Common/FltPortProcessMessage.hpp"
#include "ProcessUtils.hpp"

#include <CppSemantics.hpp>

Minifilter::ProcessFilter::ProcessFilter()
{
    auto status = ::PsSetCreateProcessNotifyRoutineEx(&ProcessCreateNotifyRoutine, false);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("::PsSetCreateProcessNotifyRoutineEx failed with status 0x%x", status);
        Invalidate();
        return;
    }
    Validate();
}

Minifilter::ProcessFilter::~ProcessFilter()
{
    if (IsValid())
    {
        auto status = ::PsSetCreateProcessNotifyRoutineEx(&ProcessCreateNotifyRoutine, true);
        NT_VERIFY(NT_SUCCESS(status));
    }
}

void
Minifilter::ProcessFilter::ProcessCreateNotifyRoutine(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
    auto rundownAcquired = ::ExAcquireRundownProtection(&gDrvData.RundownProtection);
    if (!rundownAcquired)
    {
        MyDriverLogWarning("ExAcquireRundownProtection failed at ProcessCreateNotifyRoutine");
        return;
    }    
    
    (CreateInfo != nullptr) ? HandleProcessCreate(Process, ProcessId, CreateInfo)
                            : HandleProcessTerminate(Process, ProcessId);

    ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
}

bool 
Minifilter::ProcessFilter::IsActionMonitored(
    _In_ HANDLE ProcessId,
    _Inout_opt_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorStarted))
    {
        return false;
    }

    if (CreateInfo && !gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorProcessCreate))
    {
        return false;
    }

    if (!CreateInfo && !gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorProcessTerminate))
    {
        return false;
    }

    if (ProcessUtils::IsSystemOrIdleProcess(ProcessId))
    {
        return false;
    }

    return true;
}

void
Minifilter::ProcessFilter::HandleProcessCreate(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId,
    _Inout_ PPS_CREATE_NOTIFY_INFO CreateInfo
)
{
    UNREFERENCED_PARAMETER(Process);

    Cpp::String imagePath;
    Cpp::String commandLine;

    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);

    if (CreateInfo->ImageFileName)
    {
        imagePath = Cpp::String{ (const unsigned __int8*)CreateInfo->ImageFileName->Buffer, CreateInfo->ImageFileName->Length };
    }

    if (CreateInfo->CommandLine)
    {
        commandLine = Cpp::String{ (const unsigned __int8*)CreateInfo->CommandLine->Buffer, CreateInfo->CommandLine->Length };
    }

    gDrvData.ProcessColector->AddProcess(timestamp, imagePath , ProcessId);

    if (!IsActionMonitored(ProcessId, CreateInfo))
    {
        return;
    }

    auto status = gDrvData.CommunicationPort->Send<KmUmShared::ProcessCreateMessage>(
        ProcessId, 
        timestamp, 
        HandleToULong(CreateInfo->ParentProcessId), 
        commandLine
    );

    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send process create message failed with status 0x%x", status);
    }
}

void 
Minifilter::ProcessFilter::HandleProcessTerminate(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId
)
{
    UNREFERENCED_PARAMETER(Process);

    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);

    if (!IsActionMonitored(ProcessId, nullptr))
    {
        return;
    }

    auto status = gDrvData.CommunicationPort->Send<KmUmShared::ProcessTerminateMessage>(ProcessId,timestamp);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send process create message failed with status 0x%x", status);
    }
}
