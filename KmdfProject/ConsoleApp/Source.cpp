#include <Windows.h>
#include <iostream>
#include <thread>
#include <mutex>

#define FILEIO_TYPE 40001

#define IOCTL_DIRECT CTL_CODE(FILEIO_TYPE, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_REVERSE CTL_CODE(FILEIO_TYPE, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)

std::mutex gConsoleLock;

void
IoctlSendIoctl(
    _In_ HANDLE DeviceHandle,
    _In_ DWORD IoctlCode,
    _In_opt_ LPVOID InputBuffer,
    _In_ DWORD InputBufferSize,
    _Out_opt_ LPVOID OutputBuffer,
    _In_ DWORD OutputBufferSize
);

void SendDirectCtl(_In_ HANDLE DeviceHandle);
void SendReverseCtl(_In_ HANDLE DeviceHandle);

int main()
{
    auto deviceHandle = CreateFileW(L"\\\\.\\MyDeviceSymlink\\file.file", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
    if (deviceHandle == INVALID_HANDLE_VALUE)
    {
        std::wcout << "CreateFile failed! GLE=" << GetLastError() << std::endl;
        throw std::exception("CreateFile failed");
    }

    std::thread t1{ [&]() {SendReverseCtl(deviceHandle); } };
    Sleep(1000);

    std::thread t2{ [&]() {SendDirectCtl(deviceHandle); } };
    Sleep(1000);
    
    t1.join();
    t2.join();

    CloseHandle(deviceHandle);
    return 0;
}

void
IoctlSendIoctl(
    _In_ HANDLE DeviceHandle,
    _In_ DWORD IoctlCode,
    _In_opt_ LPVOID InputBuffer,
    _In_ DWORD InputBufferSize,
    _Out_opt_ LPVOID OutputBuffer,
    _In_ DWORD OutputBufferSize
)
{
    DWORD noBytes = 0;
    OVERLAPPED overlapped = { 0 };

    overlapped.hEvent = CreateEventW(nullptr, true, false, nullptr);
    if (!overlapped.hEvent)
    {
        gConsoleLock.lock();
        std::wcout << "CreateEventW failed! GLE=" << GetLastError() << std::endl;
        gConsoleLock.unlock();

        return;
    }

    auto res = DeviceIoControl(DeviceHandle, IoctlCode, InputBuffer, InputBufferSize, OutputBuffer, OutputBufferSize, &noBytes, &overlapped);
    if (GetLastError() == ERROR_IO_PENDING)
    {
        res = WaitForSingleObject(overlapped.hEvent, INFINITE);
    }

    gConsoleLock.lock();
    std::wcout << "DeviceIoControl result = " << res << std::endl;
    gConsoleLock.unlock();

    CloseHandle(overlapped.hEvent);
}

void SendDirectCtl(_In_ HANDLE DeviceHandle)
{
    wchar_t buffer[] = L"Ana are mere";

    gConsoleLock.lock();
    std::wcout << "Sending IOCTL_DIRECT" << std::endl;
    gConsoleLock.unlock();

    IoctlSendIoctl(DeviceHandle, (DWORD)(IOCTL_DIRECT), buffer, sizeof(buffer) - sizeof(wchar_t), nullptr, 0);

    gConsoleLock.lock();
    std::wcout << "Sent " << buffer << std::endl;
    gConsoleLock.unlock();
}

void SendReverseCtl(_In_ HANDLE DeviceHandle)
{
    wchar_t buffer[1000] = { 0 };

    gConsoleLock.lock();
    std::wcout << "Sending IOCTL_REVERSE" << std::endl;
    gConsoleLock.unlock();

    IoctlSendIoctl(DeviceHandle, (DWORD)(IOCTL_REVERSE), nullptr, 0, buffer, sizeof(buffer));
    
    gConsoleLock.lock();
    std::wcout << "Received " << buffer << std::endl;
    gConsoleLock.unlock();
}
