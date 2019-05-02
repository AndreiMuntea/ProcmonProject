#include "RegistryFilter.hpp"
#include "trace.hpp"
#include "RegistryFilter.tmh"

#include "GlobalData.hpp"
#include "ProcessUtils.hpp"

#include <CppSemantics.hpp>
#include "../Common/FltPortRegistryMessage.hpp"
#include "../Common/FltPortFileMessage.hpp"

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

bool 
Minifilter::RegistryFilter::IsActionMonitored(
    _In_ REG_NOTIFY_CLASS RegistryOperationType
)
{
    auto currentPid = PsGetCurrentProcessId();

    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorStarted))
    {
        return false;
    }

    if (!IsRegistryOperationTypeAllowed(RegistryOperationType))
    {
        return false;
    }

    if (ProcessUtils::IsSystemOrIdleProcess(currentPid))
    {
        return false;
    }

    return true;
}

bool 
Minifilter::RegistryFilter::IsRegistryOperationTypeAllowed(
    _In_ REG_NOTIFY_CLASS RegistryOperationType
)
{
    switch (RegistryOperationType)
    {
    case(RegNtPostCreateKey):
    case(RegNtPreCreateKeyEx):
    case(RegNtPostCreateKeyEx):
        return gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorRegistryCreateKey);
    case(RegNtPreSetValueKey):
    case(RegNtPostSetValueKey):
        return gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorRegistrySetValue);
    case (RegNtPreDeleteKey):
    case (RegNtPostDeleteKey):
        return gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorRegistryDeleteKey);
    case (RegNtPreDeleteValueKey):
    case (RegNtPostDeleteValueKey):
        return gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorRegistryDeleteKeyValue);
    case (RegNtPreLoadKey):
    case (RegNtPostLoadKey):
        return gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorRegistryRenameKey);
    case (RegNtPreRenameKey):
    case (RegNtPostRenameKey):
        return gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorRegistryRenameKey);
    default:
        return false;
    }
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
Minifilter::RegistryFilter::RegistryHandlePreCreateKeyEx(
    _Inout_ PREG_CREATE_KEY_INFORMATION Parameters
)
{
    Cpp::String root;
    Cpp::String newKey{ (const unsigned __int8*)Parameters->CompleteName->Buffer, Parameters->CompleteName->Length };

    auto status = RegistrySolveKeyName(Parameters->RootObject, root);
    if (!NT_SUCCESS(status) || !root.IsValid() || !newKey.IsValid())
    {
        MyDriverLogCritical("Failed to retrieve root key name");
        return;
    }

    root += newKey;
    status = RegistryRegisterCallContext<RegistryKeyContext>(&Parameters->CallContext, Cpp::Forward<Cpp::String>(root));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("Failed to register registry context");
        return;
    }
}

void 
Minifilter::RegistryFilter::RegistryHandlePostCreateKey(
    _In_ unsigned __int32 ProcessId,
    _In_ unsigned __int64 Timestamp,
    _Inout_ PREG_POST_CREATE_KEY_INFORMATION Parameters
)
{
    Cpp::String name{ (const unsigned __int8*)Parameters->CompleteName->Buffer, Parameters->CompleteName->Length };

    auto status = gDrvData.CommunicationPort->Send<KmUmShared::RegistryCreateMessage>((HANDLE)ProcessId, Timestamp, name,  Parameters->Status);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send create key message failed with status 0x%x", status);
    }
}

void 
Minifilter::RegistryFilter::RegistryHandlePostSetValue(
    _In_ unsigned __int32 ProcessId,
    _In_ unsigned __int64 Timestamp,
    _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
)
{
    auto context = (RegistryKeyValueDataContext*)(Parameters->CallContext);
    if (!context)
    {
        return;
    }

    // Send generic set value message
    gDrvData.CommunicationPort->Send<KmUmShared::RegistrySetValueMessage>((HANDLE)ProcessId, Timestamp, context->keyName, context->valueName, context->data, context->dataType, Parameters->Status);

    // Check for delete at reboot
    WCHAR value[] = { L"PendingFileRenameOperations" };
    WCHAR key[] = { L"\\REGISTRY\\MACHINE\\SYSTEM\\ControlSet001\\Control\\Session Manager" };

    Cpp::String strValue{ (const unsigned __int8*)value, sizeof(value) - sizeof(WCHAR)};
    Cpp::String strKey{ (const unsigned __int8*)key, sizeof(key) - sizeof(WCHAR) };

    // Operator == will perform case insensitive comparation
    if (context->keyName == strKey && strValue == context->valueName)
    {
        gDrvData.CommunicationPort->Send<KmUmShared::FileDeleteMessage>((HANDLE)ProcessId, Timestamp, context->data, Parameters->Status, KmUmShared::FileDeleteType::PendingRegistry);
    }

    delete context;
}

void 
Minifilter::RegistryFilter::RegistryHandlePreOperationKey(
    _In_ PVOID Object,
    _Inout_ PVOID* CallContext
)
{
    *CallContext = nullptr;
    Cpp::String keyName;

    auto status = RegistrySolveKeyName(Object, keyName);
    if (!NT_SUCCESS(status) || !keyName.IsValid())
    {
        return;
    }

    status = RegistryRegisterCallContext<RegistryKeyContext>(CallContext, Cpp::Forward<Cpp::String>(keyName));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("Failed to register registry context");
        return;
    }
}

void 
Minifilter::RegistryFilter::RegistryHandlePreOperationKeyValue(
    _In_ PVOID Object,
    _In_ PCUNICODE_STRING Value,
    _Inout_ PVOID* CallContext
)
{
    *CallContext = nullptr;
    Cpp::String keyName;
    Cpp::String value{ (const unsigned __int8*)Value->Buffer, Value->Length };

    auto status = RegistrySolveKeyName(Object, keyName);
    if (!NT_SUCCESS(status) || !keyName.IsValid() || !value.IsValid())
    {
        return;
    }

    status = RegistryRegisterCallContext<RegistryKeyValueContext>(CallContext, Cpp::Forward<Cpp::String>(keyName), Cpp::Forward<Cpp::String>(value));
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("Failed to register registry context");
        return;
    }
}

void Minifilter::RegistryFilter::RegistryHandlePreOperationKeyValueData(
    _In_ PVOID Object,
    _In_ PCUNICODE_STRING Value,
    _In_ PVOID Data,
    _In_ ULONG DataSize,
    _In_ ULONG DataType,
    _Inout_ PVOID* CallContext
)
{
    Cpp::String keyName;
    Cpp::String value{ (const unsigned __int8*)Value->Buffer, Value->Length };
    Cpp::String data{ (const unsigned __int8*)Data, DataSize};

    auto status = RegistrySolveKeyName(Object, keyName);
    if (!NT_SUCCESS(status) || !keyName.IsValid() || !value.IsValid() || !data.IsValid())
    {
        return;
    }

    status = RegistryRegisterCallContext<RegistryKeyValueDataContext>(CallContext, Cpp::Forward<Cpp::String>(keyName), Cpp::Forward<Cpp::String>(value), Cpp::Forward<Cpp::String>(data), DataType);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("Failed to register registry context");
        return;
    }
}

NTSTATUS 
Minifilter::RegistryFilter::RegistryNotifyRoutine(
    _In_ PVOID CallbackContext,
    _In_ PVOID Argument1,
    _In_opt_ PVOID Argument2
)
{
    UNREFERENCED_PARAMETER(CallbackContext);

    auto rundownAcquired = ::ExAcquireRundownProtection(&gDrvData.RundownProtection);
    if (!rundownAcquired)
    {
        MyDriverLogWarning("ExAcquireRundownProtection failed at RegistryNotifyRoutine");
        return STATUS_SUCCESS;
    }

    REG_NOTIFY_CLASS notifyClass = (REG_NOTIFY_CLASS)(SIZE_T)Argument1;
    if (!IsActionMonitored(notifyClass) || !Argument2)
    {
        ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
        return STATUS_SUCCESS;
    }

    HANDLE currentPid = PsGetCurrentProcessId();
    unsigned __int32 processId = HandleToULong(currentPid);
    unsigned __int64 timestamp = 0;

    KeQuerySystemTime(&timestamp);

    switch (notifyClass)
    {
        case(RegNtPostCreateKey):
        {
            RegistryHandlePostCreateKey(processId, timestamp, (PREG_POST_CREATE_KEY_INFORMATION)Argument2);
            break;
        }
        case(RegNtPreCreateKeyEx):
        {
            RegistryHandlePreCreateKeyEx((PREG_CREATE_KEY_INFORMATION)Argument2);
            break;
        }
        case(RegNtPostCreateKeyEx):
        {
            RegistryHandlePostKeyContextMessage<KmUmShared::RegistryCreateMessage>(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
            break;
        }
        case(RegNtPreSetValueKey):
        {
            auto parameters = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;
            RegistryHandlePreOperationKeyValueData(parameters->Object, parameters->ValueName, parameters->Data, parameters->DataSize, parameters->Type, &parameters->CallContext);
            break;
        }
        case(RegNtPostSetValueKey):
        {
            RegistryHandlePostSetValue(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
            break;
        }
        case (RegNtPreDeleteKey):
        {
            auto parameters = (PREG_DELETE_KEY_INFORMATION)Argument2;
            RegistryHandlePreOperationKey(parameters->Object, &parameters->CallContext);
            break;
        }
        case (RegNtPostDeleteKey):
        {
            RegistryHandlePostKeyContextMessage<KmUmShared::RegistryDeleteKeyMessage>(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
            break;
        }
        case (RegNtPreDeleteValueKey):
        {
            auto parameters = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;
            RegistryHandlePreOperationKeyValue(parameters->Object, parameters->ValueName, &parameters->CallContext);
            break;
        }
        case (RegNtPostDeleteValueKey):
        {
            RegistryHandlePostKeyValueContextMessage<KmUmShared::RegistryDeleteKeyValueMessage>(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
            break;
        }
        case (RegNtPreLoadKey):
        {
            auto parameters = (PREG_LOAD_KEY_INFORMATION)Argument2;
            RegistryHandlePreOperationKey(parameters->Object, &parameters->CallContext);
            break;
        }
        case (RegNtPostLoadKey):
        {
            RegistryHandlePostKeyContextMessage<KmUmShared::RegistryLoadKeyMessage>(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
            break;
        }
        case (RegNtPreRenameKey):
        {
            auto parameters = (PREG_RENAME_KEY_INFORMATION)Argument2;
            RegistryHandlePreOperationKeyValue(parameters->Object, parameters->NewName, &parameters->CallContext);
            break;
        }
        case (RegNtPostRenameKey):
        {
            RegistryHandlePostKeyValueContextMessage<KmUmShared::RegistryRenameKeyMessage>(processId, timestamp, (PREG_POST_OPERATION_INFORMATION)Argument2);
            break;
        }
        default:
        {
            break;
        }
    }

    ::ExReleaseRundownProtection(&gDrvData.RundownProtection);
    return STATUS_SUCCESS;
}

Minifilter::RegistryKeyContext::RegistryKeyContext(
    Cpp::String && KeyName
) : keyName {Cpp::Forward<Cpp::String>(KeyName)}
{
}

Minifilter::RegistryKeyValueContext::RegistryKeyValueContext(
    Cpp::String && KeyName, 
    Cpp::String && ValueName
) : keyName{Cpp::Forward<Cpp::String>(KeyName)},
    valueName{Cpp::Forward<Cpp::String>(ValueName)}
{
}

Minifilter::RegistryKeyValueDataContext::RegistryKeyValueDataContext(
    Cpp::String && KeyName, 
    Cpp::String && ValueName, 
    Cpp::String && Data,
    ULONG DataType
) : keyName{Cpp::Forward<Cpp::String>(KeyName)},
    valueName{Cpp::Forward<Cpp::String>(ValueName)},
    data{Cpp::Forward<Cpp::String>(Data)},
    dataType{DataType}
{
}
