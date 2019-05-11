#ifndef __NETWORK_FILTER_HPP__ 
#define __NETWORK_FILTER_HPP__

#ifndef NDIS_SUPPORT_NDIS6
#define NDIS_SUPPORT_NDIS6 1
#endif 

#ifndef INITGUID
#define INITGUID 
#endif 

#pragma warning(push)
#pragma warning (disable : 4201)
#pragma warning (disable : 4067)

#include <CppUniquePointer.hpp>
#include "cpp_allocator_object.hpp"
#include "DeviceObject.hpp"

#include <ndis.h>
#include <wdm.h>
#include <fwpsk.h>
#include <guiddef.h>
#include <fwpmk.h>
#pragma warning(pop)


namespace Minifilter
{
    class NetworkEngine : public Cpp::CppNonPagedObject<'TEN#'>
    {
        friend class NetworkFilter;
    public:
        NetworkEngine();
        virtual ~NetworkEngine();

    private:
        HANDLE engineHandle = nullptr;
    };

    class NetworkFilter : public Cpp::CppNonPagedObject<'TFN#'>
    {
    public:
        NetworkFilter(PDRIVER_OBJECT DriverObject, PUNICODE_STRING DeviceName);
        virtual ~NetworkFilter();

        static void NTAPI
        ClassifyFn(
            _In_ const FWPS_INCOMING_VALUES0* FixedValues,
            _In_ const FWPS_INCOMING_METADATA_VALUES0* MetaValues,
            _Inout_opt_ void* LayerData,
            _In_opt_ const void* ClassifyContext,
            _In_ const FWPS_FILTER2* Filter,
            _In_ UINT64 FlowContext,
            _Inout_ FWPS_CLASSIFY_OUT0* Classify
        );

        static NTSTATUS NTAPI
        NotifyFn(
            _In_ FWPS_CALLOUT_NOTIFY_TYPE NotifyType,
            _In_ const GUID  *FilterKey,
            _Inout_ FWPS_FILTER2  *Filter
        );

        static void NTAPI
        FlowDeleteFn(
           _In_ UINT16  LayerId,
           _In_ UINT32  CalloutId,
           _In_ UINT64  FlowContext
        );

    private:
        bool RegisterAuthConnectIpV4Callout();
        bool RegisterAuthRecvAcceptIpV4Callout();

        NTSTATUS 
        RegisterCallback(
            _In_ const GUID* LayerKey,
            _In_ const GUID* CalloutKey,
            _Out_ UINT32* FwpsCalloutId,
            _Out_ UINT32* FwpmCalloutId,
            _Out_ UINT64* FilterId
        );

        NTSTATUS
        RegisterFwpsCallout(
            _In_ const GUID* LayerKey,
            _In_ const GUID* CalloutKey,
            _Out_ UINT32* FwpsCalloutId
        );

        NTSTATUS
        RegisterFwpmCallout(
            _In_ const GUID* LayerKey,
            _In_ const GUID* CalloutKey,
            _Out_ UINT32* FwpmCalloutId
        );

        NTSTATUS 
        RegisterFilter(
            _In_ const GUID* LayerKey,
            _In_ const GUID* CalloutKey,
            _Out_ UINT64* FilterId
        );

        Cpp::UniquePointer<DeviceObject> deviceObject;
        Cpp::UniquePointer<NetworkEngine> engine;

        bool authConnectIpV4CalloutRegistered = false;
        UINT32 authConnectIpV4CalloutFwpsId = 0;
        UINT32 authConnectIpV4CalloutFwpmId = 0;
        UINT64 authConnectIpV4CalloutFilterId = 0;

        bool authRecvAcceptIpV4CalloutRegistered = false;
        UINT32 authRecvAcceptIpV4CalloutFwpsId = 0;
        UINT32 authRecvAcceptIpV4CalloutFwpmId = 0;
        UINT64 authRecvAcceptIpV4CalloutFilterId = 0;
    };
};

#endif //__NETWORK_FILTER_HPP__