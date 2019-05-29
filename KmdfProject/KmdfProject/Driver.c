#include <ntddk.h>
#include <wdf.h>

#include "trace.h"
#include "Driver.tmh"

#define FILEIO_TYPE 40001
#define IOCTL_DIRECT CTL_CODE(FILEIO_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REVERSE CTL_CODE(FILEIO_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

static UNICODE_STRING gWdfDeviceName = RTL_CONSTANT_STRING(L"\\Device\\MyDeviceName");
static UNICODE_STRING gWdfSymLinkName = RTL_CONSTANT_STRING(L"\\DosDevices\\MyDeviceSymLink");
static UNICODE_STRING gCompletionMessage = RTL_CONSTANT_STRING(L"ACK");

typedef struct _DEVICE_CONTEXT {
    WDFQUEUE    NotificationQueue;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT; 
WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

DRIVER_INITIALIZE DriverEntry;

NTSTATUS
DriverSetupControlDevice(
    _In_ WDFDRIVER Driver
);

VOID
DriverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
);

NTSTATUS
DriverEvtSolveDirectRequest(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength
);

NTSTATUS
DriverEvtSolvePendRequest(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength
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

NTSTATUS
DriverEntry(
    _Inout_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    WDF_DRIVER_CONFIG configuration = { 0 };
    WDFDRIVER wdfDriver = NULL;
    WDF_OBJECT_ATTRIBUTES attributes = { 0 };
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    WDF_DRIVER_CONFIG_INIT(&configuration, WDF_NO_EVENT_CALLBACK);
    configuration.DriverInitFlags |= WdfDriverInitNonPnpDriver;
    configuration.EvtDriverUnload = DriverUnload;

    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    attributes.EvtCleanupCallback = DriverEvtObjectContextCleanup;
    attributes.EvtDestroyCallback = DriverEvtObjectContextDestroy;

    status = WdfDriverCreate(DriverObject, RegistryPath, &attributes, &configuration, &wdfDriver);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    WPP_INIT_TRACING(DriverObject, RegistryPath);
    return DriverSetupControlDevice(wdfDriver);
}


NTSTATUS
DriverSetupControlDevice(
    _In_ WDFDRIVER Driver
)
{
    PWDFDEVICE_INIT wdfDevice = NULL; 
    WDFDEVICE controlDevice = { 0 };
    WDF_OBJECT_ATTRIBUTES attributes = { 0 };
    WDF_IO_QUEUE_CONFIG ioQueueConfig = { 0 };
    DEVICE_CONTEXT* context = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    wdfDevice = WdfControlDeviceInitAllocate(Driver, &SDDL_DEVOBJ_SYS_ALL_ADM_RWX_WORLD_RW_RES_R);
    if (!wdfDevice)
    {
        MyDriverLogError("WdfControlDeviceInitAllocate failed with status 0x%X", status);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // More than one application at time can communicate with this device
    WdfDeviceInitSetExclusive(wdfDevice, FALSE);

    // Communication will be buffered
    WdfDeviceInitSetIoType(wdfDevice, WdfDeviceIoBuffered);

    // Assign NT name to device
    status = WdfDeviceInitAssignName(wdfDevice, &gWdfDeviceName);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfDeviceInitAssignName failed with status 0x%X", status);
        goto CleanUp;
    }

    // Initialize object attributes where we will store a notification queue
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
    WDF_OBJECT_ATTRIBUTES_SET_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&wdfDevice, &attributes, &controlDevice);
    if (!NT_SUCCESS(status)) 
    {
        MyDriverLogError("WdfDeviceCreate failed with status 0x%X", status);
        goto CleanUp;
    }

    // Initialize context from the control device
    context = DeviceGetContext(controlDevice);
    RtlZeroMemory(context, sizeof(DEVICE_CONTEXT));

    // Assign a symlink to control device
    status = WdfDeviceCreateSymbolicLink(controlDevice, &gWdfSymLinkName);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfDeviceCreateSymbolicLink failed with status 0x%X", status);
        goto CleanUp;
    }

    // Create default queue
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig, WdfIoQueueDispatchSequential);
    ioQueueConfig.EvtIoDeviceControl = DriverEvtIoDeviceControl;

    status = WdfIoQueueCreate(controlDevice, &ioQueueConfig, &attributes, WDF_NO_HANDLE);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfIoQueueCreate default queue failed with status 0x%X", status);
        goto CleanUp;
    }

    // Create another queue in manual mode which will be used to pend notifications
    WDF_IO_QUEUE_CONFIG_INIT(&ioQueueConfig, WdfIoQueueDispatchManual);

    status = WdfIoQueueCreate(controlDevice, &ioQueueConfig, WDF_NO_OBJECT_ATTRIBUTES, &context->NotificationQueue);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfIoQueueCreate notification queue failed with status 0x%X", status);
        goto CleanUp;
    }

    // Everything went fine :)
    WdfControlFinishInitializing(controlDevice);

CleanUp:
    if (!NT_SUCCESS(status))
    {
        WdfDeviceInitFree(wdfDevice);
    }

    return status;
}

VOID
DriverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
)
{
    MyDriverLogTrace("We are in the DriverEvtIoDeviceControl routine");
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;

    switch (IoControlCode)
    {
        case IOCTL_DIRECT:
        {
            MyDriverLogTrace("IOCTL_DIRECT");
            status = DriverEvtSolveDirectRequest(Queue, Request, OutputBufferLength, InputBufferLength);
            break;
        }
        case IOCTL_REVERSE:
        {
            MyDriverLogTrace("IOCTL_REVERSE");
            status = DriverEvtSolvePendRequest(Queue, Request, OutputBufferLength, InputBufferLength);

            if (NT_SUCCESS(status))
            {
                MyDriverLogTrace("Successfully pended request");
                return;
            }

            break;
        }
        default:
        {
            break;
        }
    }

    WdfRequestComplete(Request, status);
}


VOID
DriverUnload(
    _In_ WDFDRIVER Driver
)
{
    UNREFERENCED_PARAMETER(Driver);
    MyDriverLogTrace("We are in the DriverUnload routine");
}

NTSTATUS
DriverEvtSolveDirectRequest(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength
)
{
    UNREFERENCED_PARAMETER(OutputBufferLength);

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    WDFREQUEST pendedRequest = { 0 };
    DEVICE_CONTEXT* context = NULL;
    
    PVOID outputBuffer = NULL;
    size_t outputBufferLength = 0;

    PVOID inputBuffer = NULL;
    size_t inputBufferLength = 0;

    context = DeviceGetContext(WdfIoQueueGetDevice(Queue));
    if (!context)
    {
        MyDriverLogError("InvertedGetContextFromDevice returned NULL");
        return STATUS_INVALID_DEVICE_OBJECT_PARAMETER;
    } 
    
    if (InputBufferLength == 0)
    {
        MyDriverLogError("Input buffer is too small");
        return STATUS_INVALID_BUFFER_SIZE;
    }

    status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, &inputBuffer, &inputBufferLength);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfRequestRetrieveInputBuffer failed with status 0x%X", status);
        return status;
    }

    if (gCompletionMessage.Length + inputBufferLength < inputBufferLength)
    {
        MyDriverLogError("Input buffer is too big");
        return STATUS_BUFFER_OVERFLOW;
    }

    status = WdfIoQueueRetrieveNextRequest(context->NotificationQueue, &pendedRequest);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfIoQueueRetrieveNextRequest failed with status 0x%X", status);
        return status;
    }

    UNICODE_STRING temp = { 0 };
    temp.Buffer = (PWCHAR)(inputBuffer);
    temp.Length = (USHORT)(inputBufferLength);
    temp.MaximumLength = (USHORT)(inputBufferLength);
    MyDriverLogTrace("Input buffer received = %wZ", &temp);

    status = WdfRequestRetrieveOutputBuffer(pendedRequest, gCompletionMessage.Length + inputBufferLength, &outputBuffer, &outputBufferLength);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfRequestRetrieveOutputBuffer failed with status 0x%X", status);
        goto CleanUp;
    }

    RtlCopyMemory(outputBuffer, inputBuffer, inputBufferLength);
    RtlCopyMemory((BYTE*)outputBuffer + inputBufferLength, gCompletionMessage.Buffer, gCompletionMessage.Length);
    status = STATUS_SUCCESS;

CleanUp:
    WdfRequestCompleteWithInformation(pendedRequest, status, gCompletionMessage.Length + inputBufferLength);
    return status;
}

NTSTATUS
DriverEvtSolvePendRequest(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength
)
{
    UNREFERENCED_PARAMETER(InputBufferLength);
    DEVICE_CONTEXT* context = NULL;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    context = DeviceGetContext(WdfIoQueueGetDevice(Queue));
    if (!context)
    {
        MyDriverLogError("InvertedGetContextFromDevice returned NULL");
        return STATUS_INVALID_DEVICE_OBJECT_PARAMETER;
    }

    if (OutputBufferLength < gCompletionMessage.Length)
    {
        MyDriverLogError("Output buffer is too small");
        return STATUS_BUFFER_TOO_SMALL;
    }

    status = WdfRequestForwardToIoQueue(Request, context->NotificationQueue);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfRequestForwardToIoQueue failed with status 0x%X", status);
        return status;
    }

    return STATUS_SUCCESS;
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
    WPP_CLEANUP(WdfDriverWdmGetDriverObject((WDFDRIVER)Object));
}