#include "DeviceObject.hpp"
#include "trace.hpp"
#include "DeviceObject.tmh"

Minifilter::DeviceObject::DeviceObject(PDRIVER_OBJECT DriverObject, PUNICODE_STRING DeviceName)
{
    auto status = ::IoCreateDevice(
        DriverObject, 
        0, 
        DeviceName,
        FILE_DEVICE_UNKNOWN, 
        FILE_DEVICE_SECURE_OPEN, 
        false, 
        &deviceObject
    );

    if (!NT_SUCCESS(status))
    {
        deviceObject = nullptr;
        MyDriverLogError("::IoCreateDevice has failed with status 0x%x", status);
        Invalidate();
        return;
    }

    Validate();
}

Minifilter::DeviceObject::~DeviceObject()
{
    if (deviceObject)
    {
        ::IoDeleteDevice(deviceObject);
        deviceObject = nullptr;
    }
}

PDEVICE_OBJECT Minifilter::DeviceObject::GetDeviceObject()
{
    return this->deviceObject;
}
