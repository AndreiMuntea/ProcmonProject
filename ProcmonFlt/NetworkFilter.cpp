#include "NetworkFilter.hpp"
#include "trace.hpp"
#include "NetworkFilter.tmh"


#include "GlobalData.hpp"
#include "../Common/FltPortNetworkMessage.hpp"

// {BD2F8319-7363-4AF9-8FB2-D5CA63CB24A3}
static const GUID gAuthConnectIpV4GUID =
{ 0xbd2f8319, 0x7363, 0x4af9,{ 0x8f, 0xb2, 0xd5, 0xca, 0x63, 0xcb, 0x24, 0xa3 } };

// {AAE86F30-9BC2-4858-B9AD-FFD49D1D32F1}
static const GUID gAuthRecvAcceptIpv4GUID =
{ 0xaae86f30, 0x9bc2, 0x4858,{ 0xb9, 0xad, 0xff, 0xd4, 0x9d, 0x1d, 0x32, 0xf1 } };


Minifilter::NetworkFilter::NetworkFilter(PDRIVER_OBJECT DriverObject, PUNICODE_STRING DeviceName)
{
    this->deviceObject.Update(new DeviceObject(DriverObject, DeviceName));
    if (!deviceObject.IsValid() || !deviceObject->IsValid())
    {
        return;
    }

    this->engine.Update(new NetworkEngine());
    if (!engine.IsValid() || !engine->IsValid())
    {
        return;
    }

    if (!RegisterAuthConnectIpV4Callout())
    {
        return;
    }

    if (!RegisterAuthRecvAcceptIpV4Callout())
    {
        return;
    }

    Validate();
}

Minifilter::NetworkFilter::~NetworkFilter()
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    if (this->authConnectIpV4CalloutRegistered)
    {
        status = FwpmFilterDeleteById(this->engine->engineHandle, this->authConnectIpV4CalloutFilterId);
        NT_VERIFY(NT_SUCCESS(status));

        status = FwpmCalloutDeleteById(this->engine->engineHandle, this->authConnectIpV4CalloutFwpmId);
        NT_VERIFY(NT_SUCCESS(status));

        status = FwpsCalloutUnregisterById(this->authConnectIpV4CalloutFwpsId);
        NT_VERIFY(NT_SUCCESS(status));
    }

    if (this->authRecvAcceptIpV4CalloutRegistered)
    {
        status = FwpmFilterDeleteById(this->engine->engineHandle, this->authRecvAcceptIpV4CalloutFilterId);
        NT_VERIFY(NT_SUCCESS(status));

        status = FwpmCalloutDeleteById(this->engine->engineHandle, this->authRecvAcceptIpV4CalloutFwpmId);
        NT_VERIFY(NT_SUCCESS(status));

        status = FwpsCalloutUnregisterById(this->authRecvAcceptIpV4CalloutFwpsId);
        NT_VERIFY(NT_SUCCESS(status));
    }

    this->deviceObject.Update(nullptr);
    this->engine.Update(nullptr);
}

NTSTATUS 
Minifilter::NetworkFilter::NotifyFn(
    _In_ FWPS_CALLOUT_NOTIFY_TYPE NotifyType, 
    _In_ const GUID * FilterKey, 
    _Inout_ FWPS_FILTER2 * Filter
)
{
    UNREFERENCED_PARAMETER(NotifyType);
    UNREFERENCED_PARAMETER(FilterKey);
    UNREFERENCED_PARAMETER(Filter);

    return STATUS_SUCCESS;
}

void 
Minifilter::NetworkFilter::FlowDeleteFn(
    _In_ UINT16 LayerId,
    _In_ UINT32 CalloutId,
    _In_ UINT64 FlowContext
)
{
    UNREFERENCED_PARAMETER(LayerId);
    UNREFERENCED_PARAMETER(CalloutId);
    UNREFERENCED_PARAMETER(FlowContext);

}

bool Minifilter::NetworkFilter::RegisterAuthConnectIpV4Callout()
{
    GUID guid = FWPM_LAYER_ALE_AUTH_CONNECT_V4;
    auto status = RegisterCallback(
        &guid,
        &gAuthConnectIpV4GUID,
        Minifilter::NetworkFilter::ClassifyFn<FWPS_LAYER_ALE_AUTH_CONNECT_V4>,
        Minifilter::NetworkFilter::NotifyFn,
        Minifilter::NetworkFilter::FlowDeleteFn,
        &this->authConnectIpV4CalloutFwpsId,
        &this->authConnectIpV4CalloutFwpmId,
        &this->authConnectIpV4CalloutFilterId
    );

    return NT_SUCCESS(status);
}

bool Minifilter::NetworkFilter::RegisterAuthRecvAcceptIpV4Callout()
{
    GUID guid = FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4;
    auto status = RegisterCallback(
        &guid,
        &gAuthRecvAcceptIpv4GUID,
        Minifilter::NetworkFilter::ClassifyFn<FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4>,
        Minifilter::NetworkFilter::NotifyFn,
        Minifilter::NetworkFilter::FlowDeleteFn,
        &this->authRecvAcceptIpV4CalloutFwpsId,
        &this->authRecvAcceptIpV4CalloutFwpmId,
        &this->authRecvAcceptIpV4CalloutFilterId
    );

    return NT_SUCCESS(status);
}

NTSTATUS 
Minifilter::NetworkFilter::RegisterCallback(
    _In_ const GUID* LayerKey,
    _In_ const GUID* CalloutKey,
    _In_ FWPS_CALLOUT_CLASSIFY_FN2 ClassifyFunction,
    _In_ FWPS_CALLOUT_NOTIFY_FN2 NotifyFunction,
    _In_ FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
    _Out_ UINT32* FwpsCalloutId,
    _Out_ UINT32* FwpmCalloutId,
    _Out_ UINT64* FilterId
)
{
    *FwpsCalloutId = 0;
    *FwpmCalloutId = 0;

    auto status = RegisterFwpsCallout(LayerKey, CalloutKey, ClassifyFunction, NotifyFunction, FlowDeleteFunction, FwpsCalloutId);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("RegisterFwpsCallout failed with status 0x%x", status);
        return status;
    }

    status = RegisterFwpmCallout(LayerKey, CalloutKey, FwpsCalloutId);
    if (!NT_SUCCESS(status))
    {
        FwpsCalloutUnregisterById(*FwpsCalloutId);

        MyDriverLogCritical("RegisterFwpmCallout failed with status 0x%x", status);
        return status;
    }

    status = RegisterFilter(LayerKey, CalloutKey, FilterId);
    if (!NT_SUCCESS(status))
    {
        FwpmCalloutDeleteById(this->engine->engineHandle, *FwpmCalloutId);
        FwpsCalloutUnregisterById(*FwpsCalloutId);

        MyDriverLogCritical("RegisterFilter for callout failed with status 0x%x", status);
        return status;
    }

    return status;
}

NTSTATUS 
Minifilter::NetworkFilter::RegisterFwpsCallout(
    _In_ const GUID* LayerKey,
    _In_ const GUID* CalloutKey,
    _In_ FWPS_CALLOUT_CLASSIFY_FN2 ClassifyFunction,
    _In_ FWPS_CALLOUT_NOTIFY_FN2 NotifyFunction,
    _In_ FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
    _Out_ UINT32* FwpsCalloutId
)
{
    UNREFERENCED_PARAMETER(LayerKey);
    FWPS_CALLOUT fwpsCallout = { 0 };

    fwpsCallout.calloutKey = *CalloutKey;
    fwpsCallout.classifyFn = ClassifyFunction;
    fwpsCallout.notifyFn = NotifyFunction;
    fwpsCallout.flowDeleteFn = FlowDeleteFunction;

    return ::FwpsCalloutRegister(this->deviceObject->GetDeviceObject(), &fwpsCallout, FwpsCalloutId);
}

NTSTATUS 
Minifilter::NetworkFilter::RegisterFwpmCallout(
    _In_ const GUID* LayerKey,
    _In_ const GUID* CalloutKey,
    _Out_ UINT32* FwpmCalloutId
)
{
    FWPM_CALLOUT fwpmCallout = { 0 };
    FWPM_DISPLAY_DATA displayData = { 0 };

    displayData.name = L"Dummy Name";
    displayData.description = L"Dummy Description";

    fwpmCallout.calloutKey = *CalloutKey;
    fwpmCallout.displayData = displayData;
    fwpmCallout.applicableLayer = *LayerKey;

    return ::FwpmCalloutAdd(this->engine->engineHandle, &fwpmCallout, nullptr, FwpmCalloutId);
}

NTSTATUS 
Minifilter::NetworkFilter::RegisterFilter(
    _In_ const GUID* LayerKey,
    _In_ const GUID* CalloutKey,
    _Out_ UINT64* FilterId
)
{
    FWPM_FILTER filter = { 0 };

    filter.layerKey = *LayerKey;
    filter.displayData.name = L"Dummy filter name";
    filter.displayData.description = L"Dummy Description";

    filter.action.type = FWP_ACTION_CALLOUT_INSPECTION;
    filter.action.calloutKey = *CalloutKey;
    filter.weight.type = FWP_EMPTY;

    return FwpmFilterAdd(this->engine->engineHandle, &filter, nullptr, FilterId);
}

bool 
Minifilter::NetworkFilter::GetNetworkTupleIndexesForLayer(
    _In_ UINT16 LayerId,
    _Out_ UINT* AppIdIndex,
    _Out_ UINT* LocalAddressIndex,
    _Out_ UINT* RemoteAddressIndex,
    _Out_ UINT* LocalPortIndex,
    _Out_ UINT* RemotePortIndex,
    _Out_ UINT* ProtocolIndex,
    _Out_ UINT* IcmpIndex
)
{
    switch (LayerId)
    {
    case FWPS_LAYER_ALE_AUTH_CONNECT_V4:
        *AppIdIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_ALE_APP_ID;
        *LocalAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS;
        *RemoteAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_ADDRESS;
        *LocalPortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT;
        *RemotePortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_REMOTE_PORT;
        *ProtocolIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_PROTOCOL;
        *IcmpIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V4_ICMP_TYPE;
        return true;
    case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V4:
        *AppIdIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_ALE_APP_ID;
        *LocalAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_LOCAL_ADDRESS;
        *RemoteAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_REMOTE_ADDRESS;
        *LocalPortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_LOCAL_PORT;
        *RemotePortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_REMOTE_PORT;
        *ProtocolIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_IP_PROTOCOL;
        *IcmpIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V4_ICMP_TYPE;
        return true;
    default:
        *AppIdIndex = MAXUINT32;
        *LocalAddressIndex = MAXUINT32;
        *RemoteAddressIndex = MAXUINT32;
        *LocalPortIndex = MAXUINT32;
        *RemotePortIndex = MAXUINT32;
        *ProtocolIndex = MAXUINT32;
        *IcmpIndex = MAXUINT32;
        NT_ASSERT(0);
        return false;
    }
}

void 
Minifilter::NetworkFilter::ProcessValues(
    _In_ FWP_VALUE0& AppId,
    _In_ FWP_VALUE0& LocalAddress,
    _In_ FWP_VALUE0& RemoteAddress,
    _In_ FWP_VALUE0& LocalPort,
    _In_ FWP_VALUE0& RemotePort,
    _In_ FWP_VALUE0& Protocol,
    _In_ FWP_VALUE0& Icmp,
    _In_ HANDLE ProcessId
)
{
    if (AppId.type != FWP_BYTE_BLOB_TYPE)
    {
        MyDriverLogError("Invalid type for application id");
        return;
    }

    if (LocalAddress.type != FWP_UINT32 || RemoteAddress.type != FWP_UINT32)
    {
        MyDriverLogError("Invalid type for address");
        return;
    }

    if (LocalPort.type != FWP_UINT16 || LocalPort.type != FWP_UINT16)
    {
        MyDriverLogError("Invalid type for port");
        return;
    }

    if (Protocol.type != FWP_UINT8)
    {
        MyDriverLogError("Invalid type for protocol");
        return;
    }

    if (Icmp.type != FWP_UINT16)
    {
        MyDriverLogError("Invalid type for ICMP");
        return;
    }

    UNICODE_STRING appIdBlob{ 0 };
    appIdBlob.Buffer = (PWCHAR)AppId.byteBlob->data;
    appIdBlob.Length = static_cast<USHORT>(AppId.byteBlob->size);
    appIdBlob.MaximumLength = static_cast<USHORT>(AppId.byteBlob->size);

    MyDriverLogTrace("New data available: "
        "AppId = %wZ LocalAddress = 0x%x RemoteAddress = 0x%x LocalPort = 0x%x RemotePort = 0x%x, Protocol = 0x%x ICMP = 0x%x",
        &appIdBlob,
        LocalAddress.uint32, 
        RemoteAddress.uint32,
        LocalPort.uint16, 
        RemotePort.uint16,
        Protocol.uint8,
        Icmp.uint16
    );

    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);

    gDrvData.CommunicationPort->Send<KmUmShared::NetworkMessage>(
        ProcessId, 
        timestamp, 
        AppId.byteBlob->data,
        AppId.byteBlob->size,
        LocalAddress.uint32,
        RemoteAddress.uint32,
        LocalPort.uint16,
        RemotePort.uint16,
        Protocol.uint8,
        Icmp.uint16
    );
}


Minifilter::NetworkEngine::NetworkEngine()
{
    auto status = ::FwpmEngineOpen(nullptr, RPC_C_AUTHN_WINNT, nullptr, nullptr, &this->engineHandle);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::FwpmEngineOpen has failed with status 0x%x", status);
        return;
    }

    Validate();
}

Minifilter::NetworkEngine::~NetworkEngine()
{
    if (IsValid())
    {
        auto status = ::FwpmEngineClose(this->engineHandle);
        NT_VERIFY(NT_SUCCESS(status));
    }
}
