#ifndef __PROCESS_UTILS_HPP__
#define __PROCESS_UTILS_HPP__

void 
PuDumpActiveProcessesToolHelp32Snapshot();

void 
PuDumpActiveProcessesPsapi();

void 
PuDumpActiveProcessesZwQuerySystemInformation();

void
PuDeleteFileAtReboot();

#endif //__PROCESS_UTILS_HPP__