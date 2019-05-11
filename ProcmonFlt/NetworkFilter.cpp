#include "NetworkFilter.hpp"
#include "trace.hpp"
#include "NetworkFilter.tmh"


#include "GlobalData.hpp"

// {BD2F8319-7363-4AF9-8FB2-D5CA63CB24A3}
static const GUID gAuthConnectGUID =
{ 0xbd2f8319, 0x7363, 0x4af9,{ 0x8f, 0xb2, 0xd5, 0xca, 0x63, 0xcb, 0x24, 0xa3 } };

// {AAE86F30-9BC2-4858-B9AD-FFD49D1D32F1}
static const GUID gAuthRecvAcceptGUID =
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

void 
Minifilter::NetworkFilter::ClassifyFn(
    _In_ const FWPS_INCOMING_VALUES0* FixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES0* MetaValues,
    _Inout_opt_ void* LayerData,
    _In_opt_ const void* ClassifyContext,
    _In_ const FWPS_FILTER2* Filter,
    _In_ UINT64 FlowContext,
    _Inout_ FWPS_CLASSIFY_OUT0* Classify
)
{
    UNREFERENCED_PARAMETER(FixedValues);
    UNREFERENCED_PARAMETER(MetaValues);
    UNREFERENCED_PARAMETER(LayerData);
    UNREFERENCED_PARAMETER(Filter);
    UNREFERENCED_PARAMETER(FlowContext);
    UNREFERENCED_PARAMETER(Classify);
    UNREFERENCED_PARAMETER(ClassifyContext);

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
        &gAuthConnectGUID,
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
        &gAuthRecvAcceptGUID,
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
    _Out_ UINT32* FwpsCalloutId,
    _Out_ UINT32* FwpmCalloutId,
    _Out_ UINT64* FilterId
)
{
    *FwpsCalloutId = 0;
    *FwpmCalloutId = 0;

    auto status = RegisterFwpsCallout(LayerKey, CalloutKey, FwpsCalloutId);
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
    _Out_ UINT32* FwpsCalloutId
)
{
    UNREFERENCED_PARAMETER(LayerKey);
    FWPS_CALLOUT fwpsCallout = { 0 };

    fwpsCallout.calloutKey = *CalloutKey;
    fwpsCallout.classifyFn = Minifilter::NetworkFilter::ClassifyFn;
    fwpsCallout.notifyFn = Minifilter::NetworkFilter::NotifyFn;
    fwpsCallout.flowDeleteFn = Minifilter::NetworkFilter::FlowDeleteFn;

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
