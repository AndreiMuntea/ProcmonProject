#ifndef __NT_DEFS_CUSTOM_HPP__
#define __NT_DEFS_CUSTOM_HPP__

#include <fltKernel.h>

#ifndef PROCESS_QUERY_INFORMATION
#define PROCESS_QUERY_INFORMATION          (0x0400)
#endif 

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation = 0,
    SystemPerformanceInformation = 2,
    SystemTimeOfDayInformation = 3,
    SystemProcessInformation = 5,
    SystemProcessorPerformanceInformation = 8,
    SystemInterruptInformation = 23,
    SystemExceptionInformation = 33,
    SystemRegistryQuotaInformation = 37,
    SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

typedef
NTSTATUS
(NTAPI *PFUNC_ZwQuerySystemInformation)(
    _In_       SYSTEM_INFORMATION_CLASS     SystemInformationClass,
    _Inout_    PVOID                        SystemInformation,
    _In_       ULONG                        SystemInformationLength,
    _Out_opt_  PULONG                       ReturnLength
    );

typedef
NTSTATUS
(NTAPI *PFUNC_ZwQueryInformationProcess)(
    _In_      HANDLE           ProcessHandle,
    _In_      PROCESSINFOCLASS ProcessInformationClass,
    _Out_     PVOID            ProcessInformation,
    _In_      ULONG            ProcessInformationLength,
    _Out_opt_ PULONG           ReturnLength
    );

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    CHAR           Reserved1[16];
    PVOID          Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB {
    CHAR Reserved1[2];
    CHAR BeingDebugged;
    CHAR Reserved2[21];
    PVOID LoaderData;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    CHAR Reserved3[520];
    PVOID PostProcessInitRoutine;
    CHAR Reserved4[136];
    ULONG SessionId;
} PEB;

#endif //__NT_DEFS_CUSTOM_HPP__