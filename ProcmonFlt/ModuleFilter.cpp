#include "ModuleFilter.hpp"
#include "trace.hpp"
#include "ModuleFilter.tmh"

#include "GlobalData.hpp"
#include "../Common/FltPortModuleMessage.hpp"
#include "ProcessUtils.hpp"

#include <CppSemantics.hpp>

Minifilter::ModuleFilter::ModuleFilter()
{
    auto status = ::PsSetLoadImageNotifyRoutine(&LoadImageNotifyRoutine);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("::PsSetLoadImageNotifyRoutine failed with status 0x%x", status);
        Invalidate();
        return;
    }

    Validate();
}

Minifilter::ModuleFilter::~ModuleFilter()
{
    if (IsValid())
    {
        auto status = ::PsRemoveLoadImageNotifyRoutine(&ModuleFilter::LoadImageNotifyRoutine);
        NT_VERIFY(NT_SUCCESS(status));
    }
}

void Minifilter::ModuleFilter::LoadImageNotifyRoutine(
    _In_opt_ PUNICODE_STRING FullImageName,
    _In_ HANDLE ProcessId,
    _In_ PIMAGE_INFO ImageInfo
)
{
    auto rundownAcquired = ::ExAcquireRundownProtection(&gDrvData.RundownProtection);
    if (!rundownAcquired)
    {
        MyDriverLogWarning("ExAcquireRundownProtection failed at LoadImageNotifyRoutine");
        return;
    }   

    if (!IsActionMonitored(ProcessId))
    {
        ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
        return;
    }
    
    if (!FullImageName || !FullImageName->Buffer)
    {
        ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
        return;
    }
    
    Cpp::String imageName{ (const unsigned __int8*)FullImageName->Buffer, FullImageName->Length };

    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);
    
    auto status = gDrvData.CommunicationPort->Send<KmUmShared::ModuleMessage>(ProcessId, timestamp, (SIZE_T)ImageInfo->ImageBase, ImageInfo->ImageSize, imageName);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send image notify message failed with status 0x%x", status);
    }

    ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
}

bool 
Minifilter::ModuleFilter::IsActionMonitored(
    _In_ HANDLE ProcessId
)
{
    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorStarted))
    {
        return false;
    }

    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorImageLoaded))
    {
        return false;
    }

    if (ProcessUtils::IsSystemOrIdleProcess(ProcessId))
    {
        return false;
    }
    return true;
}
