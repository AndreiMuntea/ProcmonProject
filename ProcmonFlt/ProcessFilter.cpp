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

    auto status = CheckForRemoteShell(Process, ProcessId);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogWarning("Send process create message failed with status 0x%x", status);
    }

    if (!IsActionMonitored(ProcessId, CreateInfo))
    {
        return;
    }

    status = gDrvData.CommunicationPort->Send<KmUmShared::ProcessCreateMessage>(
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

NTSTATUS 
Minifilter::ProcessFilter::CheckForRemoteShell(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessId
)
{
    HANDLE processHandle = nullptr;
    OBJECT_ATTRIBUTES objectAttributes = { 0 };
    CLIENT_ID clientId = { 0 };
    PROCESS_BASIC_INFORMATION information = { 0 };

    InitializeObjectAttributes(&objectAttributes, 0, OBJ_KERNEL_HANDLE, 0, 0);
    clientId.UniqueProcess = ProcessId;
    clientId.UniqueThread = 0;

    auto status = ::ZwOpenProcess(&processHandle, PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, &objectAttributes, &clientId);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::ZwOpenProcess failed with status 0x%x", status);
        return status;
    }

    status = gDrvData.ZwQueryInformationProcess(processHandle, ProcessBasicInformation, &information, sizeof(information), nullptr);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("ZwQueryInformationProcess failed with status 0x%x", status);
        goto CleanUp;
    }

    status = CheckPebStandardHandles(Process, processHandle, information.PebBaseAddress);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("CheckPebStandardHandles failed with status 0x%x", status);
        goto CleanUp;
    }

CleanUp:
    ZwClose(processHandle);
    return status;
}

NTSTATUS 
Minifilter::ProcessFilter::CheckPebStandardHandles(
    _Inout_ PEPROCESS Process,
    _In_ HANDLE ProcessHandle,
    _In_ PPEB Peb
)
{
    KAPC_STATE apcState = { 0 };
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    KeStackAttachProcess(Process, &apcState);

    __try
    {
        ProbeForRead(Peb, sizeof(PEB) , 1);
        ProbeForRead(Peb->ProcessParameters, sizeof(RTL_USER_PROCESS_PARAMETERS), 1);
        MyDriverLogTrace("Peb->ProcessParameters address = 0x%p STDIN = 0x%X STDOUT = 0x%X STDERR = 0x%X", 
            Peb->ProcessParameters, 
            (DWORD)(SIZE_T)Peb->ProcessParameters->StandardInput,
            (DWORD)(SIZE_T)Peb->ProcessParameters->StandardOutput,
            (DWORD)(SIZE_T)Peb->ProcessParameters->StandardError
        );

        if (IsSocketHandle(ProcessHandle, Peb->ProcessParameters->StandardInput))
        {
            MyDriverLogTrace("STDIN is a socket handle");
        }

        if (IsSocketHandle(ProcessHandle, Peb->ProcessParameters->StandardOutput))
        {
            MyDriverLogTrace("STDOUT is a socket handle");
        }

        if (IsSocketHandle(ProcessHandle, Peb->ProcessParameters->StandardError))
        {
            MyDriverLogTrace("STDERR is a socket handle");
        }

        status = STATUS_SUCCESS;
    }
    __except ((GetExceptionCode() == STATUS_ACCESS_VIOLATION || GetExceptionCode() == STATUS_DATATYPE_MISALIGNMENT) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
    {
        status = GetExceptionCode();
    }

    KeUnstackDetachProcess(&apcState);
    return status;
}


bool 
Minifilter::ProcessFilter::IsSocketHandle(
    _In_ HANDLE ProcessHandle,
    _In_ HANDLE Handle
)
{
    HANDLE kernelHandle = nullptr;
    IO_STATUS_BLOCK statusBlock = { 0 };
    FILE_FS_DEVICE_INFORMATION deviceInfo = { 0 }; 
    PFILE_OBJECT fileObject = nullptr;
    UNICODE_STRING afd = RTL_CONSTANT_STRING(L"\\Driver\\AFD");

    auto status = ::ZwDuplicateObject(ProcessHandle, Handle, ProcessHandle, &kernelHandle, 0, OBJ_KERNEL_HANDLE, DUPLICATE_SAME_ACCESS);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::ZwDuplicateObject failed with status = 0x%X", status);
        return false;
    }

    status = ::ZwQueryVolumeInformationFile(kernelHandle, &statusBlock, &deviceInfo, sizeof(FILE_FS_DEVICE_INFORMATION), FileFsDeviceInformation);
    if (!NT_SUCCESS(status) || !NT_SUCCESS(statusBlock.Status))
    {
        MyDriverLogError("::ZwQueryVolumeInformationFile failed with status = 0x%X IOSB.status = 0x%X", status, statusBlock.Status);
        ::ZwClose(kernelHandle);
        return false;
    }

    if (deviceInfo.DeviceType != FILE_DEVICE_NAMED_PIPE)
    {
        ::ZwClose(kernelHandle);
        return false;
    }

    status = ::ObReferenceObjectByHandle(kernelHandle, 0, *IoFileObjectType, KernelMode, (PVOID*)&fileObject, nullptr);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::ObReferenceObjectByHandle failed with status = 0x%X", status);
        ::ZwClose(kernelHandle);
        return false;
    }

    MyDriverLogError("fileObject->DeviceObject->DriverObject->DriverName = %wZ", &fileObject->DeviceObject->DriverObject->DriverName);
    auto compareResult = RtlCompareUnicodeString(&fileObject->DeviceObject->DriverObject->DriverName, &afd, true);

    ::ObfDereferenceObject(fileObject);
    ::ZwClose(kernelHandle);

    return compareResult == 0;
}

