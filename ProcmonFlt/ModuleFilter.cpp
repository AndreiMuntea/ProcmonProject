#include "ModuleFilter.hpp"
#include "trace.hpp"
#include "ModuleFilter.tmh"

#include "GlobalData.hpp"
#include "../Common/FltPortModuleMessage.hpp"
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
    
    unsigned __int32 processId = HandleToULong(ProcessId);
    unsigned __int64 imageBase = (SIZE_T)ImageInfo->ImageBase;
    unsigned __int64 imageSize = ImageInfo->ImageSize;
    unsigned __int64 timestamp = 0;
    Cpp::Stream stream;

    KeQuerySystemTime(&timestamp);
    
    stream << KmUmShared::ModuleMessage(processId, timestamp, imageBase, imageSize, (const unsigned __int8*)FullImageName->Buffer, FullImageName->Length);
    auto status = gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send image notify message failed with status 0x%x", status);
    }

    ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
}
