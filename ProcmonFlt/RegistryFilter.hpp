#ifndef __REGISTRY_FILTER_HPP__ 
#define __REGISTRY_FILTER_HPP__

#include "cpp_allocator_object.hpp"
#include "FltPort.hpp"
#include <CppString.hpp>

namespace Minifilter
{
    class RegistryFilter : public Cpp::CppNonPagedObject<'TFR#'>
    {
    public:
        RegistryFilter();
        virtual ~RegistryFilter();

    private:
        
        static NTSTATUS
        RegistrySolveKeyName(
            _In_ PVOID Object,
            _Inout_ Cpp::String& KeyName
        );

        static void 
        RegistryHandlePostCreateKey(
            _In_ unsigned __int32 ProcessId,
            _In_ unsigned __int64 Timestamp,
            _Inout_ PREG_POST_CREATE_KEY_INFORMATION Parameters
        );

        static void 
        RegistryHandlePostCreateKeyEx(
            _In_ unsigned __int32 ProcessId,
            _In_ unsigned __int64 Timestamp,
            _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
        );

        static NTSTATUS
        RegistryNotifyRoutine(
            _In_ PVOID CallbackContext,
            _In_opt_ PVOID Argument1,
            _In_opt_ PVOID Argument2
        );
    };
}

#endif //__REGISTRY_FILTER_HPP__