#ifndef __REGISTRY_FILTER_HPP__ 
#define __REGISTRY_FILTER_HPP__

#include "cpp_allocator_object.hpp"
#include "FltPort.hpp"

#include <CppString.hpp>
#include <CppStream.hpp>
#include <CppSemantics.hpp>

namespace Minifilter
{
    class RegistryKeyContext : public Cpp::CppPagedObject<'TFR'>
    {
        friend class RegistryFilter;
    public:
        RegistryKeyContext(Cpp::String&& KeyName);
        virtual ~RegistryKeyContext() = default;

    private:
        Cpp::String keyName;
    };

    class RegistryKeyValueContext : public Cpp::CppPagedObject<'TFR'>
    {
        friend class RegistryFilter;
    public:
        RegistryKeyValueContext(Cpp::String&& KeyName, Cpp::String&& ValueName);
        virtual ~RegistryKeyValueContext() = default;

    private:
        Cpp::String keyName;
        Cpp::String valueName;
    };

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
        RegistryHandlePreCreateKeyEx(
            _Inout_ PREG_CREATE_KEY_INFORMATION Parameters
        );

        static void 
        RegistryHandlePostCreateKey(
            _In_ unsigned __int32 ProcessId,
            _In_ unsigned __int64 Timestamp,
            _Inout_ PREG_POST_CREATE_KEY_INFORMATION Parameters
        );

        static void 
        RegistryHandlePreOperationKeyValue(
            _In_ PVOID Object,
            _In_ PCUNICODE_STRING Value,
            _Inout_ PVOID* CallContext
        );

        static NTSTATUS
        RegistryNotifyRoutine(
            _In_ PVOID CallbackContext,
            _In_opt_ PVOID Argument1,
            _In_opt_ PVOID Argument2
        );

        template <class T>
        static RegistryHandlePostKeyContextMessage(
            _In_ unsigned __int32 ProcessId,
            _In_ unsigned __int64 Timestamp,
            _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
        );

        template <class T>
        static RegistryHandlePostKeyValueContextMessage(
            _In_ unsigned __int32 ProcessId,
            _In_ unsigned __int64 Timestamp,
            _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
        );

        template <class Context, class ...Args>
        static NTSTATUS 
        RegistryRegisterCallContext(
            _Out_ PVOID* CallContext,
            Args&&... Arguments
        );
    };
    template<class T>
    inline RegistryFilter::RegistryHandlePostKeyContextMessage(
        _In_ unsigned __int32 ProcessId,
        _In_ unsigned __int64 Timestamp,
        _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
    )
    {
        auto context = (RegistryKeyContext*)(Parameters->CallContext);
        if (context)
        {
            Cpp::Stream stream;
            stream << T{ Timestamp, ProcessId, context->keyName.GetNakedPointer(), context->keyName.GetSize(), Parameters->Status };
            gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));

            delete context;
            Parameters->CallContext = nullptr;
        }
    }

    template<class T>
    inline RegistryFilter::RegistryHandlePostKeyValueContextMessage(
        _In_ unsigned __int32 ProcessId,
        _In_ unsigned __int64 Timestamp,
        _Inout_ PREG_POST_OPERATION_INFORMATION Parameters
    )
    {
        auto context = (RegistryKeyValueContext*)(Parameters->CallContext);
        if (context)
        {
            Cpp::Stream stream;
            stream << T{ Timestamp, ProcessId, context->keyName.GetNakedPointer(),context->keyName.GetSize(), context->valueName.GetNakedPointer(), context->valueName.GetSize(), Parameters->Status };
            gDrvData.CommunicationPort->Send(Cpp::Forward<Cpp::Stream>(stream));
            
            delete context;
            Parameters->CallContext = nullptr;
        }
    }
    template<class Context, class ...Args>
    inline NTSTATUS RegistryFilter::RegistryRegisterCallContext(
        _Out_ PVOID* CallContext,
        Args&&... Arguments
    )
    {
        *CallContext = nullptr;

        auto context = new Context(Cpp::Forward<Args>(Arguments)...);
        if (!context)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        *CallContext = context;
        return STATUS_SUCCESS;
    }
}

#endif //__REGISTRY_FILTER_HPP__