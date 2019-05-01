#ifndef __GLOBAL_DATA_HPP__ 
#define __GLOBAL_DATA_HPP__

#include <fltKernel.h>
#include <CppUniquePointer.hpp>

#include "FltPort.hpp"
#include "ProcessFilter.hpp"
#include "ThreadFilter.hpp"
#include "ModuleFilter.hpp"
#include "RegistryFilter.hpp"
#include "ConfigurationManager.hpp"

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
    Cpp::UniquePointer<Minifilter::ModuleFilter> ModuleFilter;
    Cpp::UniquePointer<Minifilter::RegistryFilter> RegistryFilter;
    Cpp::UniquePointer<Minifilter::ConfigurationManager> ConfigurationManager;

    EX_RUNDOWN_REF RundownProtection;
}GLOBAL_DATA, *PGLOBAL_DATA;

extern GLOBAL_DATA gDrvData;

void GdrvInitGlobalData(_In_ PDRIVER_OBJECT DriverObject);
void GdrvUninitGlobalData();

#endif //__GLOBAL_DATA_HPP__