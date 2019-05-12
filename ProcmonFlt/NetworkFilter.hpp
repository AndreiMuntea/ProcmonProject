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

        template <UINT16 Layer>
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

        bool RegisterAuthConnectIpV6Callout();
        bool RegisterAuthRecvAcceptIpV6Callout();

        NTSTATUS 
        RegisterCallback(
            _In_ const GUID* LayerKey,
            _In_ const GUID* CalloutKey,
            _In_ FWPS_CALLOUT_CLASSIFY_FN2 ClassifyFunction,
            _In_ FWPS_CALLOUT_NOTIFY_FN2 NotifyFunction,
            _In_ FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
            _Out_ UINT32* FwpsCalloutId,
            _Out_ UINT32* FwpmCalloutId,
            _Out_ UINT64* FilterId
        );

        NTSTATUS
        RegisterFwpsCallout(
            _In_ const GUID* LayerKey,
            _In_ const GUID* CalloutKey,
            _In_ FWPS_CALLOUT_CLASSIFY_FN2 ClassifyFunction,
            _In_ FWPS_CALLOUT_NOTIFY_FN2 NotifyFunction,
            _In_ FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
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
        Cpp::UniquePointer<NetworkEngine> engine;

        bool authConnectIpV4CalloutRegistered = false;
        UINT32 authConnectIpV4CalloutFwpsId = 0;
        UINT32 authConnectIpV4CalloutFwpmId = 0;
        UINT64 authConnectIpV4CalloutFilterId = 0;

        bool authRecvAcceptIpV4CalloutRegistered = false;
        UINT32 authRecvAcceptIpV4CalloutFwpsId = 0;
        UINT32 authRecvAcceptIpV4CalloutFwpmId = 0;
        UINT64 authRecvAcceptIpV4CalloutFilterId = 0;

        bool authConnectIpV6CalloutRegistered = false;
        UINT32 authConnectIpV6CalloutFwpsId = 0;
        UINT32 authConnectIpV6CalloutFwpmId = 0;
        UINT64 authConnectIpV6CalloutFilterId = 0;

        bool authRecvAcceptIpV6CalloutRegistered = false;
        UINT32 authRecvAcceptIpV6CalloutFwpsId = 0;
        UINT32 authRecvAcceptIpV6CalloutFwpmId = 0;
        UINT64 authRecvAcceptIpV6CalloutFilterId = 0;
    };
    template<UINT16 Layer>
    inline void NetworkFilter::ClassifyFn(
        _In_ const FWPS_INCOMING_VALUES0* FixedValues,
        _In_ const FWPS_INCOMING_METADATA_VALUES0* MetaValues,
        _Inout_opt_ void* LayerData,
        _In_opt_ const void* ClassifyContext,
        _In_ const FWPS_FILTER2* Filter,
        _In_ UINT64 FlowContext,
        _Inout_ FWPS_CLASSIFY_OUT0* Classify
    )
    {
        UNREFERENCED_PARAMETER(MetaValues);
        UNREFERENCED_PARAMETER(LayerData);
        UNREFERENCED_PARAMETER(Filter);
        UNREFERENCED_PARAMETER(FlowContext);
        UNREFERENCED_PARAMETER(Classify);
        UNREFERENCED_PARAMETER(ClassifyContext);

        UINT AppIdIndex = 0;
        UINT LocalAddressIndex = 0;
        UINT RemoteAddressIndex = 0;
        UINT LocalPortIndex = 0;
        UINT RemotePortIndex = 0;
        UINT ProtocolIndex = 0;
        UINT IcmpIndex = 0;

        if (!GetNetworkTupleIndexesForLayer(Layer, &AppIdIndex, &LocalAddressIndex, &RemoteAddressIndex, &LocalPortIndex, &RemotePortIndex, &ProtocolIndex, &IcmpIndex))
        {
            return;
        }

        ProcessIpValues(
            FixedValues->incomingValue[AppIdIndex].value,
            FixedValues->incomingValue[LocalAddressIndex].value,
            FixedValues->incomingValue[RemoteAddressIndex].value,
            FixedValues->incomingValue[LocalPortIndex].value,
            FixedValues->incomingValue[RemotePortIndex].value,
            FixedValues->incomingValue[ProtocolIndex].value,
            FixedValues->incomingValue[IcmpIndex].value,
            ULongToHandle(static_cast<DWORD>(MetaValues->processId))
        );
    }
};

#endif //__NETWORK_FILTER_HPP__