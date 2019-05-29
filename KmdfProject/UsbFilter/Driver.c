#include <ntifs.h>
#include <ntddk.h>
#include <wdf.h>
#include <initguid.h>
#include <usbiodef.h>
#include <Wdmguid.h>
#include <intrin.h>

#include "trace.h"
#include "Driver.tmh"

#define FILEIO_TYPE 40001

#define IOCTL_ALLOW CTL_CODE(FILEIO_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DENY CTL_CODE(FILEIO_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

static UNICODE_STRING gWdfDeviceName = RTL_CONSTANT_STRING(L"\\Device\\MyDeviceName");
static UNICODE_STRING gWdfSymLinkName = RTL_CONSTANT_STRING(L"\\DosDevices\\MyDeviceSymLink");

PVOID gNotificationEntry = NULL;
volatile char gAllowConnection = FALSE;


DRIVER_INITIALIZE DriverEntry;

VOID
DriverUnload(
    _In_ WDFDRIVER Driver
);

VOID
DriverEvtObjectContextCleanup(
    _In_ WDFOBJECT Object
);

VOID
DriverEvtObjectContextDestroy(
    _In_ WDFOBJECT Object
);

NTSTATUS
DriverUsbNotificationRoutine(
    _In_ PVOID NotificationStructure,
    _Inout_opt_ PVOID Context
);

NTSTATUS
DriverEjectUsb(
    _In_ PUNICODE_STRING InterfaceName
);

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

    status = DriverSetupControlDevice(wdfDriver);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    WPP_INIT_TRACING(DriverObject, RegistryPath);
    status = IoRegisterPlugPlayNotification(
        EventCategoryDeviceInterfaceChange,
        0,
        (PVOID)(&GUID_DEVINTERFACE_USB_DEVICE),
        DriverObject,
        DriverUsbNotificationRoutine,
        NULL,
        &gNotificationEntry
    );

    if (!NT_SUCCESS(status))
    {
        WPP_CLEANUP(DriverObject);
        MyDriverLogError("IoRegisterPlugPlayNotification failed with status 0x%X", status);
    }
    return status;
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

VOID
DriverUnload(
    _In_ WDFDRIVER Driver
)
{
    UNREFERENCED_PARAMETER(Driver);
    MyDriverLogTrace("We are in the DriverUnload routine");

    NTSTATUS status = IoUnregisterPlugPlayNotificationEx(gNotificationEntry);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("IoUnregisterPlugPlayNotificationEx failed with status 0x%X", status);
        NT_ASSERT(FALSE);
    }
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

    // Initialize object attributes
    WDF_OBJECT_ATTRIBUTES_INIT(&attributes);

    status = WdfDeviceCreate(&wdfDevice, &attributes, &controlDevice);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("WdfDeviceCreate failed with status 0x%X", status);
        goto CleanUp;
    }

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

    // Everything went fine :)
    WdfControlFinishInitializing(controlDevice);

CleanUp:
    if (!NT_SUCCESS(status))
    {
        WdfDeviceInitFree(wdfDevice);
    }

    return status;
}

NTSTATUS
DriverUsbNotificationRoutine(
    _In_ PVOID NotificationStructure,
    _Inout_opt_ PVOID Context
)
{
    UNREFERENCED_PARAMETER(NotificationStructure);
    UNREFERENCED_PARAMETER(Context);

    DEVICE_INTERFACE_CHANGE_NOTIFICATION* interfaceChangeNotification = NULL;
    interfaceChangeNotification = (DEVICE_INTERFACE_CHANGE_NOTIFICATION*)NotificationStructure;
    MyDriverLogTrace("We are in the DriverUsbNotificationRoutine routine. SymbolicLinkName = %wZ", interfaceChangeNotification->SymbolicLinkName);

    // Check whether the notification is about inserting the device
    if (!RtlEqualMemory(&interfaceChangeNotification->Event, &GUID_DEVICE_INTERFACE_ARRIVAL, sizeof(GUID)))
    {
        return STATUS_SUCCESS;
    }

    // Block connection
    if (_InterlockedCompareExchange8(&gAllowConnection, FALSE, FALSE) == FALSE)
    {
        return DriverEjectUsb(interfaceChangeNotification->SymbolicLinkName);
    }

    // Allow connection
    return STATUS_SUCCESS;
}

NTSTATUS
DriverEjectUsb(
    _In_ PUNICODE_STRING InterfaceName
)
{
    PFILE_OBJECT fileObject = NULL;
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_OBJECT physicalDeviceObject = NULL;

    // Get the device object associated with interface
    NTSTATUS status = IoGetDeviceObjectPointer(InterfaceName, FILE_ALL_ACCESS, &fileObject, &deviceObject);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("IoGetDeviceObjectPointer failed with status 0x%X", status);
        return status;
    }

    // https://groups.google.com/forum/#!topic/microsoft.public.development.device.drivers/QlhqAXsTwx8
    // Call IoGetBaseFileSystemDeviceObject on the file object returned by IoGetDeviceObjectPointer to get the PDO.
    physicalDeviceObject = IoGetBaseFileSystemDeviceObject(fileObject);

    // Request the PDO to be ejected
    IoRequestDeviceEject(physicalDeviceObject);

    ObDereferenceObject(fileObject);
    return STATUS_SUCCESS;
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
    UNREFERENCED_PARAMETER(Queue);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;

    if (IoControlCode == IOCTL_ALLOW)
    {
        MyDriverLogTrace("IOCTL_ALLOW");
        _InterlockedExchange8(&gAllowConnection, TRUE);
        status = STATUS_SUCCESS;
    }
    else if (IoControlCode == IOCTL_DENY)
    {
        MyDriverLogTrace("IOCTL_DENY");
        _InterlockedExchange8(&gAllowConnection, FALSE);
        status = STATUS_SUCCESS;
    }

    WdfRequestComplete(Request, status);
}