#include <Windows.h>
#include <iostream>
#include <string>

#define FILEIO_TYPE 40001

#define IOCTL_ALLOW CTL_CODE(FILEIO_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_DENY CTL_CODE(FILEIO_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

void
IoctlSendIoctl(
    _In_ HANDLE DeviceHandle,
    _In_ DWORD IoctlCode
);

int main()
{
    auto deviceHandle = CreateFileW(L"\\\\.\\MyDeviceSymlink\\file.file", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    if (deviceHandle == INVALID_HANDLE_VALUE)
    {
        std::wcout << "CreateFile failed! GLE=" << GetLastError() << std::endl;
        throw std::exception("CreateFile failed");
    }

    std::wstring response;

    std::wcout << "Which is the default response for USB connections ? ALLOW / DENY\n";
    std::wcin >> response;

    if (response == L"ALLOW") IoctlSendIoctl(deviceHandle, IOCTL_ALLOW);
    else if (response == L"DENY") IoctlSendIoctl(deviceHandle, IOCTL_DENY);
    else std::wcout << "Invalid option!\n";

    CloseHandle(deviceHandle);
    return 0;
}

void
IoctlSendIoctl(
    _In_ HANDLE DeviceHandle,
    _In_ DWORD IoctlCode
)
{
    DWORD noBytes = 0;
    OVERLAPPED overlapped = { 0 };

    overlapped.hEvent = CreateEventW(nullptr, true, false, nullptr);
    if (!overlapped.hEvent)
    {
        std::wcout << "CreateEventW failed! GLE=" << GetLastError() << std::endl;
        return;
    }

    auto res = DeviceIoControl(DeviceHandle, IoctlCode, nullptr, 0, nullptr, 0, &noBytes, &overlapped);
    if (GetLastError() == ERROR_IO_PENDING)
    {
        res = WaitForSingleObject(overlapped.hEvent, INFINITE);
    }

    std::wcout << "DeviceIoControl result = " << res << std::endl;
    CloseHandle(overlapped.hEvent);
}