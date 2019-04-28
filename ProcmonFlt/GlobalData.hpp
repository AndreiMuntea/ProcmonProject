#ifndef __GLOBAL_DATA_HPP__ 
#define __GLOBAL_DATA_HPP__

#include <fltKernel.h>
#include <CppUniquePointer.hpp>

#include "FltPort.hpp"
#include "ProcessFilter.hpp"
#include "ThreadFilter.hpp"

typedef struct _GLOBAL_DATA
{
    PDRIVER_OBJECT DriverObject;
    PFLT_FILTER FilterHandle;
    FLT_REGISTRATION FilterRegistration;
    UNICODE_STRING Altitude;
    LARGE_INTEGER Cookie;
    
    UNICODE_STRING CommunicationPortName;
    Cpp::UniquePointer<Minifilter::FltPort> CommunicationPort;

    Cpp::UniquePointer<Minifilter::ProcessFilter> ProcessFilter;
    Cpp::UniquePointer<Minifilter::ThreadFilter> ThreadFilter;
}GLOBAL_DATA, *PGLOBAL_DATA;

extern GLOBAL_DATA gDrvData;

void GdrvInitGlobalData(_In_ PDRIVER_OBJECT DriverObject);
void GdrvUninitGlobalData();

#endif //__GLOBAL_DATA_HPP__