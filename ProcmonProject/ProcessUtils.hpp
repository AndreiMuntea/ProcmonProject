#ifndef __PROCESS_UTILS_HPP__
#define __PROCESS_UTILS_HPP__

#include <string>

void
PuDumpActiveProcessesToolHelp32Snapshot();

void
PuDumpActiveProcessesPsapi();

void
PuDumpActiveProcessesZwQuerySystemInformation();

void
PuDeleteFileAtReboot();

std::wstring
PuNtPathToDosPath(
    const std::wstring& NtPath
);

#endif //__PROCESS_UTILS_HPP__