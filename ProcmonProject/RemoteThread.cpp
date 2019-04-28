#include "RemoteThread.hpp"
#include "trace.h"
#include "RemoteThread.tmh"

#include "GlobalData.hpp"

#include <iostream>
#include <Windows.h>

void 
RmtpOpenCmd(
    _Inout_ STARTUPINFOW* StartupInfo,
    _Inout_ PROCESS_INFORMATION* ProcInfo
)
{
    LPWSTR path = L"c:\\Windows\\System32\\cmd.exe";
    auto res = CreateProcessW(
        path,                   // lpApplicationName
        nullptr,                // lpCommandLine
        nullptr,                // lpProcessAttributes
        nullptr,                // lpThreadAttributes
        true,                   // bInheritHandles
        CREATE_NEW_CONSOLE,     // dwCreationFlags
        nullptr,                // lpEnvironment
        nullptr,                // lpCurrentDirectory
        StartupInfo,            // lpStartupInfo
        ProcInfo                // lpProcessInformation
    );

    if (!res)
    {
        std::wcout << "RmtpOpenCmd failed! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("RmtpOpenCmd failed with GLE=%d", GetLastError());
        throw std::exception("RmtpOpenCmd failed");
    }

    std::wcout << "Created cmd process" << std::endl;
    Sleep(2000);
}

void 
RmtpCreateRemoteThread(
    _Inout_ PROCESS_INFORMATION* ProcInfo,
    _Inout_ HANDLE* ThreadHandle
)
{
    // ntdll should be mapped at the same address. We already resolved some functions from ntdll.
    LPTHREAD_START_ROUTINE startRoutine = (LPTHREAD_START_ROUTINE)gGlobalData.ZwQueryInformationProcess;
 
    *ThreadHandle = CreateRemoteThread(
        ProcInfo->hProcess, // hProcess
        nullptr,            // lpThreadAttributes
        0,                  // dwStackSize
        startRoutine,       // lpStartAddress
        nullptr,            // lpParameter
        0,                  // dwCreationFlags
        nullptr             // lpThreadId
    );
    
    if (*ThreadHandle)
    {
        std::wcout << "Created Remote thread" << std::endl;
        Sleep(2000);
    }
    else
    {
        std::wcout << "CreateRemoteThread failed! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("CreateRemoteThread failed with GLE=%d", GetLastError());
    }
}

void RmtpCleanupRemoteThread(
    _Inout_ HANDLE* ThreadHandle
)
{
    if (*ThreadHandle)
    {
        WaitForSingleObject(*ThreadHandle, INFINITE);
        CloseHandle(*ThreadHandle);
        *ThreadHandle = nullptr;
    }
}

void RmtpCleanupProcess(
    _Inout_ PROCESS_INFORMATION* ProcInfo,
    _Inout_ HANDLE* ThreadHandle
)
{
    TerminateProcess(ProcInfo->hProcess, 0);
    RmtpCleanupRemoteThread(ThreadHandle);

    WaitForSingleObject(ProcInfo->hProcess, INFINITE);
    CloseHandle(ProcInfo->hProcess);
    CloseHandle(ProcInfo->hThread);
}

void RmtCreateRemoteThread()
{
    STARTUPINFOW startupInfo = { 0 };
    PROCESS_INFORMATION procInfo = { 0 };
    HANDLE threadHandle = nullptr;

    RmtpOpenCmd(&startupInfo, &procInfo);
    RmtpCreateRemoteThread(&procInfo, &threadHandle);
    RmtpCleanupProcess(&procInfo, &threadHandle);
}
