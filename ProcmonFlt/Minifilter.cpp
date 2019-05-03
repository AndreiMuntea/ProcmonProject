#include <fltKernel.h>

#include "DriverRoutines.hpp"
#include "trace.hpp"
#include "Minifilter.tmh"

#include "cpp_init.hpp"
#include "GlobalData.hpp"
#include "Commands.hpp"

#include <CppStream.hpp>
#include <CppShallowStream.hpp>

EXTERN_C_START

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)
    #pragma alloc_text(PAGE, DriverUnload)
    #pragma alloc_text(PAGE, InstanceSetup)
    #pragma alloc_text(PAGE, InstanceQueryTeardown)
    #pragma alloc_text(PAGE, InstanceTeardownStart)
    #pragma alloc_text(PAGE, InstanceTeardownComplete)
#endif

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT     DriverObject,
    _In_ PUNICODE_STRING    RegistryPath
)
{
    WPP_INIT_TRACING(DriverObject, RegistryPath);
    MyDriverLogTrace("We are now in driver entry routine!");

    // Initialize Cpp Support
    Cpp::CppInitialize();

    // Initialize other globals 
    GdrvInitGlobalData(DriverObject);

    // Register filter 
    auto status = ::FltRegisterFilter(DriverObject, &gDrvData.FilterRegistration, &gDrvData.FilterHandle);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("::FltRegisterFilter failed with status 0x%x", status);
        goto Exit;
    }

    // Create Blacklist
    gDrvData.BlackList.Update(new Minifilter::BlackList());
    if (!gDrvData.BlackList.IsValid() || !gDrvData.BlackList->IsValid())
    {
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize BlackList");
        goto Exit;
    }

    // Create Process Colector
    gDrvData.ProcessColector.Update(new Minifilter::ProcessCollector());
    if (!gDrvData.ProcessColector.IsValid() || !gDrvData.ProcessColector->IsValid())
    {
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize ProcessColector");
        goto Exit;
    }

    // Create configuration manager
    gDrvData.ConfigurationManager.Update(new Minifilter::ConfigurationManager());
    if (!gDrvData.ConfigurationManager.IsValid() || !gDrvData.ConfigurationManager->IsValid())
    {
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize ConfigurationManager");
        goto Exit;
    }

    // Create a new communication port
    gDrvData.CommunicationPort.Update(new Minifilter::FltPort(gDrvData.FilterHandle, &gDrvData.CommunicationPortName, OnMessageReceived, gDrvData.ProcessColector.GetRawPointer()));
    if (!gDrvData.CommunicationPort.IsValid() || !gDrvData.CommunicationPort->IsValid())
    {
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize Filter communication port");
        goto Exit;
    }

    // Create Process filter
    gDrvData.ProcessFilter.Update(new Minifilter::ProcessFilter());
    if (!gDrvData.ProcessFilter.IsValid() || !gDrvData.ProcessFilter->IsValid())
    {
        gDrvData.CommunicationPort.Update(nullptr);
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize ProcessFilter");
        goto Exit;
    }

    // Create Thread filter
    gDrvData.ThreadFilter.Update(new Minifilter::ThreadFilter());
    if (!gDrvData.ThreadFilter.IsValid() || !gDrvData.ThreadFilter->IsValid())
    {
        gDrvData.CommunicationPort.Update(nullptr);
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize ThreadFilter");
        goto Exit;
    }

    // Create Module filter
    gDrvData.ModuleFilter.Update(new Minifilter::ModuleFilter());
    if (!gDrvData.ModuleFilter.IsValid() || !gDrvData.ModuleFilter->IsValid())
    {
        gDrvData.CommunicationPort.Update(nullptr);
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize ModuleFilter");
        goto Exit;
    }

    // Create Registry Filter
    gDrvData.RegistryFilter.Update(new Minifilter::RegistryFilter());
    if (!gDrvData.RegistryFilter.IsValid() || !gDrvData.RegistryFilter->IsValid())
    {
        gDrvData.CommunicationPort.Update(nullptr);
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("Failed to initialize RegistryFilter");
        goto Exit;
    }

    // Start filtering
    status = ::FltStartFiltering(gDrvData.FilterHandle);
    if (!NT_SUCCESS(status))
    {
        gDrvData.CommunicationPort.Update(nullptr);
        ::FltUnregisterFilter(gDrvData.FilterHandle);
        MyDriverLogCritical("::FltStartFiltering failed with status 0x%x", status);
        goto Exit;
    }

Exit:
    if (!NT_SUCCESS(status))
    {
        WPP_CLEANUP(gDrvData.DriverObject);
        GdrvUninitGlobalData();
        Cpp::CppUninitialize();
    }
    return status;
}

NTSTATUS FLTAPI
DriverUnload(
    _In_ FLT_FILTER_UNLOAD_FLAGS Flags
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(Flags);

    MyDriverLogTrace("We are now in driver unload routine!");
    WPP_CLEANUP(gDrvData.DriverObject);

    // Stop monitoring
    gDrvData.ConfigurationManager->DisableFeature(KmUmShared::Feature::featureMonitorStarted);

    // Wait for running callbacks to complete
    ::ExWaitForRundownProtectionRelease(&gDrvData.RundownProtection);
    ::ExRundownCompleted(&gDrvData.RundownProtection);

    // Release filters
    gDrvData.ThreadFilter.Update(nullptr);
    gDrvData.ProcessFilter.Update(nullptr);
    gDrvData.ModuleFilter.Update(nullptr);
    gDrvData.RegistryFilter.Update(nullptr);

    // Close communication port (has a reference to filter handle => has to be done before FltUnregisterFilter)
    gDrvData.CommunicationPort.Update(nullptr);

    // Unregister filter
    ::FltUnregisterFilter(gDrvData.FilterHandle);

    // Cleanup other globals
    GdrvUninitGlobalData();

    // Uninit Cpp support
    Cpp::CppUninitialize();

    return STATUS_SUCCESS;
}


NTSTATUS FLTAPI
InstanceSetup(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);


    MyDriverLogTrace("We are now in instance setup routine!");
    return STATUS_SUCCESS;
}

NTSTATUS FLTAPI
InstanceQueryTeardown(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);

    MyDriverLogTrace("We are now in instance query teardown routine!");
    return STATUS_SUCCESS;
}

VOID FLTAPI
InstanceTeardownStart(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Reason
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Reason);

    MyDriverLogTrace("We are now in instance teardown start routine!");
}

VOID FLTAPI
InstanceTeardownComplete(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_TEARDOWN_FLAGS Reason
)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Reason);

    MyDriverLogTrace("We are now in instance teardown complete routine!");
}

NTSTATUS 
OnMessageReceived(
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
)
{
    MyDriverLogTrace("We are now in OnMessageReceived routine!");

    *ReturnOutputBufferLength = 0;
    KmUmShared::CommandHeader commandHeader;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    Cpp::ShallowStream inputStream((unsigned __int8*)InputBuffer, InputBufferLength);
    Cpp::Stream outputStream;

    
    commandHeader.Deserialize(inputStream);
    if (!inputStream.IsValid() || commandHeader.commandCode >= KmUmShared::CommandCode::commandMaxIndex)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    switch (commandHeader.commandCode)
    {
    case KmUmShared::CommandCode::commandEnableFeature:
        status = CommandOnUpdateFeature(true, inputStream, outputStream);
        break;
    case KmUmShared::CommandCode::commandDisableFeature:
        status = CommandOnUpdateFeature(false, inputStream, outputStream);
        break;
    case KmUmShared::CommandCode::commandProtectFolder:
        status = CommandOnUpdateBlacklistedPath(true, inputStream, outputStream);
        break;
    case KmUmShared::CommandCode::commandUnprotectFolder:
        status = CommandOnUpdateBlacklistedPath(false, inputStream, outputStream);
        break;
    case KmUmShared::CommandCode::commandSetConfiguration:
        status = CommandOnSetConfiguration(inputStream, outputStream);
        break;
    default:
        return STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS(status) || !inputStream.IsValid() || !outputStream.IsValid() || outputStream.GetSize() > OutputBufferLength)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (outputStream.GetSize() != 0)
    {
        RtlCopyMemory(OutputBuffer, outputStream.GetRawData(), outputStream.GetSize());
        *ReturnOutputBufferLength = outputStream.GetSize();
    }

    return STATUS_SUCCESS;
}

EXTERN_C_END