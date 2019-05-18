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
#include <CppSharedPointer.hpp>
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
        friend class NetworkCallout;
    public:
        NetworkEngine();
        virtual ~NetworkEngine();

    private:
        HANDLE engineHandle = nullptr;
    };

    class NetworkCallout : public Cpp::CppNonPagedObject<'TEN#'>
    {
        friend class NetworkFilter;
    public:
        NetworkCallout(
            Cpp::UniquePointer<DeviceObject>& DeviceObject,
            Cpp::SharedPointer<NetworkEngine>& Engine,
            FWPS_CALLOUT_CLASSIFY_FN2 ClassifyFunction,
            FWPS_CALLOUT_NOTIFY_FN2 NotifyFunction,
            FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
            const GUID& LayerKey,
            const GUID& CalloutKey
        );
        virtual ~NetworkCallout();

    protected:
        NTSTATUS
        RegisterFwpsCallout(
            Cpp::UniquePointer<DeviceObject>& DeviceObject
        );

        NTSTATUS
        RegisterFwpmCallout(
            Cpp::UniquePointer<DeviceObject>& DeviceObject
        );

        NTSTATUS 
        RegisterFilter(
            Cpp::UniquePointer<DeviceObject>& DeviceObject
        );

        bool
        RegisterCallout(
            Cpp::UniquePointer<DeviceObject>& DeviceObject
        );

    private:    
        Cpp::SharedPointer<NetworkEngine> engine;

        UINT32 calloutFwpsId = 0;
        UINT32 calloutFwpmId = 0;
        UINT64 calloutFilterId = 0;

        FWPS_CALLOUT_CLASSIFY_FN2 classifyFunction;
        FWPS_CALLOUT_NOTIFY_FN2 notifyFunction;
        FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 flowDeleteFunction;

        GUID layerKey;
        GUID calloutKey;
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
        static bool 
        GetNetworkTupleIndexesForLayer(
            _In_ UINT16 LayerId,
            _Out_ UINT* AppIdIndex,
            _Out_ UINT* LocalAddressIndex,
            _Out_ UINT* RemoteAddressIndex,
            _Out_ UINT* LocalPortIndex,
            _Out_ UINT* RemotePortIndex,
            _Out_ UINT* ProtocolIndex,
            _Out_ UINT* IcmpIndex
        );

        static void 
        ProcessIpValues(
            _In_ const FWP_VALUE0& AppId,
            _In_ const FWP_VALUE0& LocalAddress,
            _In_ const FWP_VALUE0& RemoteAddress,
            _In_ const FWP_VALUE0& LocalPort,
            _In_ const FWP_VALUE0& RemotePort,
            _In_ const FWP_VALUE0& Protocol,
            _In_ const FWP_VALUE0& Icmp,
            _In_ HANDLE ProcessId
        );

        static void 
        ProcessIpV4Values(
            _In_ const FWP_VALUE0& AppId,
            _In_ const FWP_VALUE0& LocalAddress,
            _In_ const FWP_VALUE0& RemoteAddress,
            _In_ const FWP_VALUE0& LocalPort,
            _In_ const FWP_VALUE0& RemotePort,
            _In_ const FWP_VALUE0& Protocol,
            _In_ const FWP_VALUE0& Icmp,
            _In_ HANDLE ProcessId
        );

        static void 
        ProcessIpV6Values(
            _In_ const FWP_VALUE0& AppId,
            _In_ const FWP_VALUE0& LocalAddress,
            _In_ const FWP_VALUE0& RemoteAddress,
            _In_ const FWP_VALUE0& LocalPort,
            _In_ const FWP_VALUE0& RemotePort,
            _In_ const FWP_VALUE0& Protocol,
            _In_ const FWP_VALUE0& Icmp,
            _In_ HANDLE ProcessId
        );

        Cpp::UniquePointer<DeviceObject> deviceObject;
        Cpp::SharedPointer<NetworkEngine> engine;

        Cpp::UniquePointer<NetworkCallout> authConnectIpv4Callout;
        Cpp::UniquePointer<NetworkCallout> authRecvIpv4Callout;

        Cpp::UniquePointer<NetworkCallout> authConnectIpv6Callout;
        Cpp::UniquePointer<NetworkCallout> authRecvIpv6Callout;
    };

};

#endif //__NETWORK_FILTER_HPP__