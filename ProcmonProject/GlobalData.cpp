#include "trace.h"
#include "GlobalData.tmh"
#include "GlobalData.hpp"

#include <iostream>

GlobalData gGlobalData;



GlobalData::GlobalData()
{
    SolveImports();
}

GlobalData::~GlobalData()
{
    CleanImports();
}

void 
GlobalData::SolveImports()
{
    NtDllModuleHandle = LoadLibrary("ntdll.dll");
    if (!NtDllModuleHandle)
    {
        CleanImports();
        std::wcout << "LoadLibrary ntdll.dll has failed with GLE = " << GetLastError() << std::endl;
        ConsoleAppLogCritical("LoadLibrary ntdll.dll has failed with GLE = %d", GetLastError());
        throw std::exception("LoadLibrary ntdll.dll has failed");
    }

    ZwQueryInformationProcess = (PFUNC_ZwQueryInformationProcess)GetProcAddress(NtDllModuleHandle, "ZwQueryInformationProcess");
    if (!ZwQueryInformationProcess)
    {
        CleanImports();
        std::wcout << "GetProcAddress ZwQueryInformationProcess failed with GLE = " << GetLastError() << std::endl;
        ConsoleAppLogCritical("GetProcAddress ZwQueryInformationProcess has failed with GLE = %d", GetLastError());
        throw std::exception("GetProcAddress ZwQueryInformationProcess has failed");
    }

    ZwReadVirtualMemory = (PFUNC_ZwReadVirtualMemory)GetProcAddress(NtDllModuleHandle, "ZwReadVirtualMemory");
    if (!ZwReadVirtualMemory)
    {
        CleanImports();
        std::wcout << "GetProcAddress ZwReadVirtualMemory failed with GLE = " << GetLastError() << std::endl;
        ConsoleAppLogCritical("GetProcAddress ZwReadVirtualMemory failed with GLE = %d", GetLastError());
        throw std::exception("GetProcAddress ZwReadVirtualMemory has failed");
    }

    ZwQuerySystemInformation = (PFUNC_ZwQuerySystemInformation)GetProcAddress(NtDllModuleHandle, "ZwQuerySystemInformation");
    if (!ZwQuerySystemInformation)
    {
        CleanImports();
        std::wcout << "GetProcAddress ZwQuerySystemInformation failed with GLE = " << GetLastError() << std::endl;
        ConsoleAppLogCritical("GetProcAddress ZwQuerySystemInformation failed with GLE = %d", GetLastError());
        throw std::exception("GetProcAddress ZwQuerySystemInformation has failed");
    }
}

void GlobalData::CleanImports()
{
    if (NtDllModuleHandle)
    {
        FreeLibrary(NtDllModuleHandle);
        NtDllModuleHandle = nullptr;
    }

    if (ZwQueryInformationProcess)
    {
        ZwQueryInformationProcess = nullptr;
    }

    if (ZwReadVirtualMemory)
    {
        ZwReadVirtualMemory = nullptr;
    }

    if (ZwQuerySystemInformation)
    {
        ZwQuerySystemInformation = nullptr;
    }
}
