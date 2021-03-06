#include "ThreadFilter.hpp"
#include "trace.hpp"
#include "ThreadFilter.tmh"

#include "GlobalData.hpp"
#include "../Common/FltPortThreadMessage.hpp"
#include "ProcessUtils.hpp"

#include <CppSemantics.hpp>

Minifilter::ThreadFilter::ThreadFilter()
{
    auto status = ::PsSetCreateThreadNotifyRoutine(&ThreadFilter::CreateThreadNotifyRoutine);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("::PsSetCreateThreadNotifyRoutine failed with status 0x%x", status);
        Invalidate();
        return;
    }

    Validate();
}

Minifilter::ThreadFilter::~ThreadFilter()
{
    if (IsValid())
    {
        auto status = ::PsRemoveCreateThreadNotifyRoutine(&ThreadFilter::CreateThreadNotifyRoutine);
        NT_VERIFY(NT_SUCCESS(status));
    }
}

void 
Minifilter::ThreadFilter::CreateThreadNotifyRoutine(
    _In_ HANDLE ProcessId,
    _In_ HANDLE ThreadId,
    _In_ BOOLEAN Create
)
{
    auto rundownAcquired = ::ExAcquireRundownProtection(&gDrvData.RundownProtection);
    if (!rundownAcquired)
    {
        MyDriverLogWarning("ExAcquireRundownProtection failed at CreateThreadNotifyRoutine");
        return;
    }

    if (!IsActionMonitored(ProcessId, Create))
    {
        ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
        return;
    }

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);
    
    if (Create)
    {
        status = gDrvData.CommunicationPort->Send<KmUmShared::ThreadCreateMessage>(ProcessId, timestamp, HandleToULong(ThreadId));
    }
    else
    {
        status = gDrvData.CommunicationPort->Send<KmUmShared::ThreadTerminateMessage>(ProcessId, timestamp, HandleToULong(ThreadId));
    }

    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send thread create/terminate message failed with status 0x%x", status);
    }

    ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
}

bool 
Minifilter::ThreadFilter::IsActionMonitored(
    _In_ HANDLE ProcessId,
    _In_ BOOLEAN Create
)
{
    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorStarted))
    {
        return false;
    }

    if (Create && !gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorThreadCreate))
    {
        return false;
    }

    if (!Create && !gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorThreadTerminate))
    {
        return false;
    }

    if (ProcessUtils::IsSystemOrIdleProcess(ProcessId))
    {
        return false;
    }

    return true;
}
