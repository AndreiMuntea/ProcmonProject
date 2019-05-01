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
    
    if (!IsActionMonitored(ProcessId, CreateInfo))
    {
        ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
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
    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(Feature::featureMonitorStarted))
    {
        return false;
    }

    if (CreateInfo && !gDrvData.ConfigurationManager->IsFeatureEnabled(Feature::featureMonitorProcessCreate))
    {
        return false;
    }

    if (!CreateInfo && !gDrvData.ConfigurationManager->IsFeatureEnabled(Feature::featureMonitorProcessTerminate))
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

    unsigned __int64 timestamp = 0;
    unsigned __int32 parentId = 0;
    unsigned __int32 processId = 0;
    const unsigned __int8* imagePath = nullptr;
    unsigned __int32 imagePathSize = 0;
    const unsigned __int8* commandLine = nullptr;
    unsigned __int32 commandLineSize = 0;

    KeQuerySystemTime(&timestamp);
    processId = HandleToULong(ProcessId);
    parentId = HandleToULong(CreateInfo->ParentProcessId);

    if (CreateInfo->ImageFileName)
    {
        imagePath = (const unsigned __int8*)CreateInfo->ImageFileName->Buffer;
        imagePathSize = CreateInfo->ImageFileName->Length;
    }

    if (CreateInfo->CommandLine)
    {
        commandLine = (const unsigned __int8*)CreateInfo->CommandLine->Buffer;
        commandLineSize = CreateInfo->CommandLine->Length;
    }

    Cpp::Stream stream;
    stream << KmUmShared::ProcessCreateMessage{ timestamp, parentId, processId, imagePath, imagePathSize, commandLine, commandLineSize };

    auto status = gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));
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
    unsigned __int32 processId = 0;

    KeQuerySystemTime(&timestamp);
    processId = HandleToULong(ProcessId);

    Cpp::Stream stream;
    stream << KmUmShared::ProcessTerminateMessage{ timestamp, processId};

    auto status = gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send process create message failed with status 0x%x", status);
    }
}
