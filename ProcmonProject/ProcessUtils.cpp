#include "trace.h"
#include "ProcessUtils.tmh"
#include "ProcessUtils.hpp"
#include "GlobalData.hpp"

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <exception>
#include <string>
#include <vector>
#include <iostream>
#include <experimental/filesystem>


static void
PupDumpProcess(
    _In_ DWORD ProcessId
)
{
    // In some cases we will fail to retrieve the command line.
    // We might not have the sufficient rights to obtain the proces handle or read its virtual memory. 
    // Therefore we will only display as much information as we can.

    PROCESS_BASIC_INFORMATION basicInformation = { 0 };
    PVOID  processParametersAddress = nullptr;
    PVOID commandLineAddress = nullptr;
    SIZE_T  processParametersActualAddress = 0;
    RTL_USER_PROCESS_PARAMETERS userProcessParameters = { 0 };
    SIZE_T noBytesRead = 0;
    std::wstring commandLine;

    auto handle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, false, ProcessId);
    if (!handle)
    {
        std::wcout << "\t > Can't display more information. Open Process failed with GLE =  " << std::hex << GetLastError() << std::endl;
        return;
    }
    std::wcout << "\t > [Opened handle] " << handle << std::endl;

    // Get the peb address
    auto status = gGlobalData.ZwQueryInformationProcess(handle, ProcessBasicInformation, &basicInformation, sizeof(PROCESS_BASIC_INFORMATION), nullptr);
    if (!NT_SUCCESS(status))
    {
        std::wcout << "\t > Can't display more information. ZwQueryInformationProcess failed with NTSTATUS =  " << std::hex << status << std::endl;
        goto CleanUp;
    }
    std::wcout << "\t > [PEB address] " << basicInformation.PebBaseAddress << std::endl;
    
    // Read the process parameters address 
    processParametersAddress = (PBYTE)basicInformation.PebBaseAddress + FIELD_OFFSET(PEB, ProcessParameters);
    status = gGlobalData.ZwReadVirtualMemory(handle, processParametersAddress, &processParametersActualAddress, sizeof(PVOID), &noBytesRead);
    if (!NT_SUCCESS(status))
    {
        std::wcout << "\t > Can't display more information. ZwReadVirtualMemory failed with NTSTATUS =  " << std::hex << status << std::endl;
        goto CleanUp;
    }
    std::wcout << "\t > [Process parameters address] " << processParametersActualAddress << std::endl;

    // Read the user process parameters
    status = gGlobalData.ZwReadVirtualMemory(handle, (PVOID)processParametersActualAddress, &userProcessParameters, sizeof(RTL_USER_PROCESS_PARAMETERS), &noBytesRead);
    if (!NT_SUCCESS(status))
    {
        std::wcout << "\t > Can't display more information. ZwReadVirtualMemory failed with NTSTATUS =  " << std::hex << status << std::endl;
        goto CleanUp;
    }

    // Allocate the command line buffer
    commandLine = std::wstring(userProcessParameters.CommandLine.Length, 0);
    commandLineAddress = userProcessParameters.CommandLine.Buffer;

    // Read the actual command line
    status = gGlobalData.ZwReadVirtualMemory(handle, (PVOID)commandLineAddress, &commandLine[0], userProcessParameters.CommandLine.Length, &noBytesRead);
    if (!NT_SUCCESS(status))
    {
        std::wcout << "\t > Can't display more information. ZwReadVirtualMemory failed with NTSTATUS =  " << std::hex << status << std::endl;
        goto CleanUp;
    }

    std::wcout << "\t > [Command Line] " << commandLine << std::endl;

CleanUp:
    CloseHandle(handle);
}

void 
PuDumpActiveProcessesToolHelp32Snapshot()
{
    PROCESSENTRY32W pe32 = { 0 };

    auto snapshotHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshotHandle == INVALID_HANDLE_VALUE)
    {
        std::wcout << "CreateToolhelp32Snapshot failed! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("CreateToolhelp32Snapshot failed with GLE=%d", GetLastError());
        throw std::exception("CreateToolhelp32Snapshot failed");
    }
    ConsoleAppLogInfo("CreateToolhelp32Snapshot succeded. Parsing processes...");

    std::wcout << "Parsing processes..." << std::endl;

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(snapshotHandle, &pe32))
    {
        std::wcout << "Failed to get first process! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("Process32FirstW failed with GLE=%d", GetLastError());
        goto CleanUp;
    }

    do
    {
        std::wcout << "=====================================================================" << std::endl;
        std::wcout << "\t > [Process ID] " << pe32.th32ProcessID << std::endl;
        std::wcout << "\t > [Name] " << std::wstring(pe32.szExeFile) << std::endl;
        std::wcout << "\t > [Parent ID] " << pe32.th32ParentProcessID << std::endl;
        std::wcout << "\t > [No Threads] " << pe32.cntThreads << std::endl;

        PupDumpProcess(pe32.th32ProcessID);

    } while (Process32NextW(snapshotHandle, &pe32));

CleanUp:
    CloseHandle(snapshotHandle);
}

void PuDumpActiveProcessesPsapi()
{
    // We consider we have at most 10000 processes running
    std::vector<DWORD> processIds(10000, 0);
    DWORD noBytesReturned = 0;
    DWORD noActualProcesses = 0;
    DWORD noAllocatedBytes = static_cast<DWORD>(processIds.size()) * sizeof(DWORD);

    if (!EnumProcesses(&processIds[0], noAllocatedBytes, &noBytesReturned))
    {
        std::wcout << "EnumProcesses failed! GLE=" << GetLastError() << std::endl;
        ConsoleAppLogError("EnumProcesses failed with GLE=%d", GetLastError());
        throw std::exception("EnumProcesses failed");
    }

    noActualProcesses = noBytesReturned / sizeof(DWORD);
    for (DWORD i = 0; i < noActualProcesses; ++i)
    {   
        std::wcout << "=====================================================================" << std::endl;
        std::wcout << "\t > [Process ID] " << processIds[i] << std::endl;

        PupDumpProcess(processIds[i]);
    }
}

void 
PuDumpActiveProcessesZwQuerySystemInformation()
{
    ULONG requiredSize = 0;
    DWORD noProcesses = 0;
    SYSTEM_PROCESS_INFORMATION* processes = nullptr;
    NTSTATUS status = STATUS_SUCCESS;

    while(true)
    {
        // Get the required size
        status = gGlobalData.ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &requiredSize);
        if (!NT_SUCCESS(status) && status != STATUS_INFO_LENGTH_MISMATCH && status != STATUS_BUFFER_TOO_SMALL)
        {
            ConsoleAppLogCritical("ZwQuerySystemInformation failed with status = 0x%X", status);
            throw std::exception("ZwQuerySystemInformation has failed");
        }

        // Allocate space for 10 extra processes
        noProcesses = requiredSize / sizeof(SYSTEM_PROCESS_INFORMATION) + 10;
        processes = new SYSTEM_PROCESS_INFORMATION[noProcesses];

        if (!processes)
        {
            ConsoleAppLogCritical("Not enough memory");
            throw std::exception("Not enough memory");
        }
        ZeroMemory(processes, sizeof(*processes));

        // Fill the information
        status = gGlobalData.ZwQuerySystemInformation(SystemProcessInformation, processes, noProcesses * sizeof(SYSTEM_PROCESS_INFORMATION), &requiredSize);
        if (!NT_SUCCESS(status) && status != STATUS_INFO_LENGTH_MISMATCH && status != STATUS_BUFFER_TOO_SMALL)
        {
            delete[] processes;
            ConsoleAppLogCritical("ZwQuerySystemInformation failed with status = 0x%X", status);
            throw std::exception("ZwQuerySystemInformation has failed");
        }

        // We had enough space to get all processes
        if (NT_SUCCESS(status))
        {
            break;
        }

        delete[] processes;
    }

    for (DWORD idx = 0; idx + sizeof(SYSTEM_PROCESS_INFORMATION) < requiredSize;)
    {
        auto current = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)processes + idx);   
        auto pid = static_cast<DWORD>((SIZE_T)current->UniqueProcessId);

        std::wcout << "=====================================================================" << std::endl;
        std::wcout << "\t > [Process ID] " << pid << std::endl;

        PupDumpProcess(pid);
        
        idx += current->NextEntryOffset;
        if (current->NextEntryOffset == 0)
        {
            break;
        }
    }

    delete[] processes;
}

void PuDeleteFileAtReboot()
{
    std::wstring fileName = std::wstring(std::experimental::filesystem::current_path().c_str()) + L"dummy_file_to_be_deleted.123.txt";
    
    // create file;
    std::ofstream stream{ fileName };
    stream.close();

    if (!MoveFileExW(fileName.c_str(), nullptr, MOVEFILE_DELAY_UNTIL_REBOOT))
    {
        std::wcout << "MoveFileExW failed with GLE=" << GetLastError() << std::endl;
    }
}

std::wstring PuNtPathToDosPath(const std::wstring & NtPath)
{
    WCHAR deviceName[MAX_PATH] = { 0 };

    if (!std::experimental::filesystem::exists(NtPath))
    {
        throw std::exception("Provided path doesn't exists!");
    }

    if (!std::experimental::filesystem::is_directory(NtPath))
    {
        throw std::exception("Provided is not a directory!");
    }

    // Remove trailing slash
    std::wstring dosPath = NtPath.substr(2, NtPath.size() - 2);
    if (NtPath.back() == L'\\')
    {
        dosPath.pop_back();
    }

    // C: or D:
    auto driveLetter = NtPath.substr(0, 2);
    if (!QueryDosDeviceW(driveLetter.c_str(), deviceName, sizeof(deviceName)))
    {
        throw std::exception("QueryDosDeviceW failed!");
    }

    std::wstring device{ deviceName, sizeof(deviceName) };
    std::wcout << "Found dos device for " << driveLetter << device << std::endl;

    return device + dosPath;
}
