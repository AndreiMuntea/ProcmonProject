#ifndef __DEVICE_OBJECT_HPP__ 
#define __DEVICE_OBJECT_HPP__

#include "cpp_allocator_object.hpp"

namespace Minifilter
{
    class DeviceObject : public Cpp::CppNonPagedObject<'TVD#'>
    {
    public:
        DeviceObject(PDRIVER_OBJECT DriverObject, PUNICODE_STRING DeviceName);
        virtual ~DeviceObject();

        PDEVICE_OBJECT GetDeviceObject();

    private:
        PDEVICE_OBJECT deviceObject = nullptr;
    };
}

#endif //__DEVICE_OBJECT_HPP__