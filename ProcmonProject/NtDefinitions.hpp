#ifndef __NT_DEFINITIONS_HPP__ 
#define __NT_DEFINITIONS_HPP__ 

#include <Windows.h>
#include <fltUser.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG           Length;
    HANDLE          RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG           Attributes;
    PVOID           SecurityDescriptor;
    PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS    Status;
        PVOID       Pointer;
    }DUMMYUNIONNAME;

    ULONG_PTR       Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef enum _PROCESSINFOCLASS {
    ProcessBasicInformation = 0,
} PROCESSINFOCLASS;

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemProcessInformation = 5,
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           NumberOfThreads;
    BYTE            Reserved1[48];
    PVOID           Reserved2[3];
    HANDLE          UniqueProcessId;
    PVOID           Reserved3;
    ULONG           HandleCount;
    BYTE            Reserved4[4];
    PVOID           Reserved5[11];
    SIZE_T          PeakPagefileUsage;
    SIZE_T          PrivatePageCount;
    LARGE_INTEGER   Reserved6[6];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _PEB_LDR_DATA {
    BYTE       Reserved1[8];
    PVOID      Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE           Reserved1[16];
    PVOID          Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

// Platform dependend : https://docs.microsoft.com/en-us/windows/desktop/api/Winternl/ns-winternl-_peb
#ifndef AMD64
typedef struct _PEB {
    BYTE                          Reserved1[2];
    BYTE                          BeingDebugged;
    BYTE                          Reserved2[1];
    PVOID                         Reserved3[2];
    PPEB_LDR_DATA                 Ldr;
    PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
    PVOID                         Reserved4[3];
    PVOID                         AtlThunkSListPtr;
    PVOID                         Reserved5;
    ULONG                         Reserved6;
    PVOID                         Reserved7;
    ULONG                         Reserved8;
    ULONG                         AtlThunkSListPtr32;
    PVOID                         Reserved9[45];
    BYTE                          Reserved10[96];
    PVOID                         PostProcessInitRoutine;
    BYTE                          Reserved11[128];
    PVOID                         Reserved12[1];
    ULONG                         SessionId;
} PEB, *PPEB;
#else
typedef struct _PEB {
    BYTE                          Reserved1[2];
    BYTE                          BeingDebugged;
    BYTE                          Reserved2[21];
    PPEB_LDR_DATA                 LoaderData;
    PRTL_USER_PROCESS_PARAMETERS  ProcessParameters;
    BYTE                          Reserved3[520];
    PVOID                         PostProcessInitRoutine;
    BYTE                          Reserved4[136];
    ULONG                         SessionId;
} PEB, *PPEB;
#endif

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID       Reserved1;
    PPEB        PebBaseAddress;
    PVOID       Reserved2[2];
    ULONG_PTR   UniqueProcessId;
    PVOID       Reserved3;
} PROCESS_BASIC_INFORMATION;

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)
#endif 

#ifndef STATUS_BUFFER_TOO_SMALL
#define STATUS_BUFFER_TOO_SMALL          ((NTSTATUS)0xC0000023L)
#endif 

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#endif 

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)  (((NTSTATUS)(Status)) >= 0)
#endif 

#endif //__NT_DEFINITIONS_HPP__