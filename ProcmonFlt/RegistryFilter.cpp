#include "RegistryFilter.hpp"
#include "trace.hpp"
#include "RegistryFilter.tmh"

#include "GlobalData.hpp"
#include <CppSemantics.hpp>
#include "../Common/FltPortRegistryMessage.hpp"

Minifilter::RegistryFilter::RegistryFilter()
{
    auto status = ::CmRegisterCallbackEx(&RegistryFilter::RegistryNotifyRoutine, &gDrvData.Altitude, gDrvData.DriverObject, nullptr, &gDrvData.Cookie, nullptr);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("::CmRegisterCallbackEx failed with status 0x%x", status);
        Invalidate();
        return;
    }
    Validate();
}

Minifilter::RegistryFilter::~RegistryFilter()
{
    auto status = CmUnRegisterCallback(gDrvData.Cookie);
    NT_ASSERT(NT_SUCCESS(status));
}

NTSTATUS 
Minifilter::RegistryFilter::RegistrySolveKeyName(
    _In_ PVOID Object,
    _Inout_ Cpp::String& KeyName
)
{
    PCUNICODE_STRING name = nullptr;

    auto status = CmCallbackGetKeyObjectIDEx(&gDrvData.Cookie, Object, nullptr, &name, 0);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    KeyName = Cpp::String{ (const unsigned __int8*)name->Buffer, name->Length };
    CmCallbackReleaseKeyObjectIDEx(name);

    return STATUS_SUCCESS;
}

void 
Minifilter::RegistryFilter::RegistryHandlePostCreateKey(
    _In_ unsigned __int32 ProcessId,
    _In_ unsigned __int64 Timestamp,
    _Inout_ PREG_POST_CREATE_KEY_INFORMATION Parameters
)
{
    Cpp::Stream stream;
    KmUmShared::RegistryCreateMessage message(
        Timestamp, 
        ProcessId, 
        (const unsigned __int8*)Parameters->CompleteName->Buffer, 
        Parameters->CompleteName->Length,
        Parameters->Status
    );

    stream << message;
    auto status = gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send create key message failed with status 0x%x", status);
    }
}

void Minifilter::RegistryFilter::RegistryHandlePostCreateKeyEx(
    _In_ unsigned __int32 ProcessId,
    _In_ unsigned __int64 Timestamp,
    _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
)
{
    Cpp::String keyName;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    
    if (NT_SUCCESS(Parameters->Status))
    {
        status = RegistrySolveKeyName(Parameters->Object, keyName);
    }

    if (!NT_SUCCESS(status))
    {
        // Fallback strategy. Retrieve as much as it is possible from pre info
        auto preInformation = (PREG_CREATE_KEY_INFORMATION)Parameters->PreInformation;  
        keyName = Cpp::String{ (const unsigned __int8*)preInformation->CompleteName->Buffer, preInformation->CompleteName->Length };
    }

    Cpp::Stream stream;
    KmUmShared::RegistryCreateMessage message(
        Timestamp,
        ProcessId,
        keyName.GetNakedPointer(),
        keyName.GetSize(),
        Parameters->Status
    );

    stream << message;
    status = gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send create key message failed with status 0x%x", status);
    }
}

NTSTATUS 
Minifilter::RegistryFilter::RegistryNotifyRoutine(
    _In_ PVOID CallbackContext,
    _In_opt_ PVOID Argument1,
    _In_opt_ PVOID Argument2
)
{
    UNREFERENCED_PARAMETER(CallbackContext);

    auto rundownAcquired = ::ExAcquireRundownProtection(&gDrvData.RundownProtection);
    if (!rundownAcquired)
    {
        MyDriverLogWarning("ExAcquireRundownProtection failed at ProcessCreateNotifyRoutine");
        return STATUS_SUCCESS;
    }

    unsigned __int32 processId = HandleToULong(PsGetCurrentProcessId());
    unsigned __int64 timestamp = 0;

    KeQuerySystemTime(&timestamp);

    REG_NOTIFY_CLASS notifyClass = (REG_NOTIFY_CLASS)(SIZE_T)Argument1;
    switch (notifyClass)
    {
    case(RegNtPostCreateKey):
        RegistryHandlePostCreateKey(processId, timestamp, (PREG_POST_CREATE_KEY_INFORMATION)Argument2);
        break;
    case(RegNtPostCreateKeyEx):
        RegistryHandlePostCreateKeyEx(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
        break;
    default:
        break;
    }

    ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
    return STATUS_SUCCESS;
}
