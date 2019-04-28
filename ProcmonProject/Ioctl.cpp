#include "Ioctl.hpp"
#include "trace.h"
#include "Ioctl.tmh"

#include <Windows.h>
#include <iostream>

void 
IoctlSendIoctl(
    _In_ DWORD IoctlCode,
    _In_opt_ LPVOID InputBuffer,
    _In_ DWORD InputBufferSize
)
{
    auto deviceHandle = CreateFile(
        "\\\\.\\MyIoctlSymLink",
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (deviceHandle == INVALID_HANDLE_VALUE)
    {
        std::wcout << "CreateFile failed! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("CreateFile failed with GLE=%d", GetLastError());
        throw std::exception("CreateFile failed");
    }

    DWORD noBytes = 0;
    auto res = DeviceIoControl(deviceHandle, IoctlCode, InputBuffer, InputBufferSize, nullptr, 0, &noBytes, nullptr);
    if (!res)
    {
        std::wcout << "DeviceIoControl failed! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("DeviceIoControl failed with GLE=%d", GetLastError());
    }

    CloseHandle(deviceHandle);
}