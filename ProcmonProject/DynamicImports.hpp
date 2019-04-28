#ifndef __DYNAMIC_IMPORTS_HPP__ 
#define __DYNAMIC_IMPORTS_HPP__ 

#include "NtDefinitions.hpp"

typedef NTSTATUS(NTAPI *PFUNC_ZwQueryInformationProcess)(
    _In_      HANDLE           ProcessHandle,
    _In_      PROCESSINFOCLASS ProcessInformationClass,
    _Out_     PVOID            ProcessInformation,
    _In_      ULONG            ProcessInformationLength,
    _Out_opt_ PULONG           ReturnLength
);

typedef NTSTATUS (NTAPI *PFUNC_ZwReadVirtualMemory)(
    _In_    HANDLE               ProcessHandle,
    _In_    PVOID                BaseAddress,
    _Out_   PVOID                Buffer,
    _In_    SIZE_T               NumberOfBytesToRead,
    _Out_   PSIZE_T              NumberOfBytesRead
);

typedef NTSTATUS (NTAPI *PFUNC_ZwQuerySystemInformation)(
    _In_            SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Inout_opt_     PVOID                    SystemInformation,
    _In_            ULONG                    SystemInformationLength,
    _Out_opt_       PULONG                   ReturnLength
);

#endif //__DYNAMIC_IMPORTS_HPP__