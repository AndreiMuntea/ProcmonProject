#include "NetworkFilter.hpp"
#include "trace.hpp"
#include "NetworkFilter.tmh"

#include <ntstrsafe.h>

#include "GlobalData.hpp"
#include "../Common/FltPortNetworkMessage.hpp"

#define STREAM_EDIT_TAG_BUFFER     'BES#' // #SEB -> Stream Edit Buffer
#define STREAM_EDIT_TAG_NBL_POOL   'LBN#' // #NBL -> Net Buffer List pool

static inline void
LogBuffer(
    _In_ PVOID Buffer,
    _In_ USHORT BufferSize
)
{
    UNICODE_STRING ustr = { BufferSize, BufferSize, (PWCHAR)Buffer };
    MyDriverLogTrace("Buffer = %wZ", &ustr);
}


// {BD2F8319-7363-4AF9-8FB2-D5CA63CB24A3}
static const GUID gAuthConnectIpV4GUID =
{ 0xbd2f8319, 0x7363, 0x4af9,{ 0x8f, 0xb2, 0xd5, 0xca, 0x63, 0xcb, 0x24, 0xa3 } };

// {AAE86F30-9BC2-4858-B9AD-FFD49D1D32F1}
static const GUID gAuthRecvAcceptIpv4GUID =
{ 0xaae86f30, 0x9bc2, 0x4858,{ 0xb9, 0xad, 0xff, 0xd4, 0x9d, 0x1d, 0x32, 0xf1 } };

// {3785FE2B-264A-499C-A13A-DC86C9971078}
static const GUID gAuthConnectIpV6GUID =
{ 0x3785fe2b, 0x264a, 0x499c,{ 0xa1, 0x3a, 0xdc, 0x86, 0xc9, 0x97, 0x10, 0x78 } };

// {FAC5F63E-13F9-4043-BEAA-D2E93B7A8C7F}
static const GUID gAuthRecvAcceptIpv6GUID =
{ 0xfac5f63e, 0x13f9, 0x4043,{ 0xbe, 0xaa, 0xd2, 0xe9, 0x3b, 0x7a, 0x8c, 0x7f } };

// {FED9B0BF-919B-42E3-BF2B-B8021BE32A31}
static const GUID gStreamLayerIpV4GUID =
{ 0xfed9b0bf, 0x919b, 0x42e3,{ 0xbf, 0x2b, 0xb8, 0x2, 0x1b, 0xe3, 0x2a, 0x31 } };

// {2FA52940-0DEA-47DB-817F-4088182926C2}
static const GUID gStreamLayerIpV6GUID =
{ 0x2fa52940, 0xdea, 0x47db,{ 0x81, 0x7f, 0x40, 0x88, 0x18, 0x29, 0x26, 0xc2 } };

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

    this->injectionHandle.Update(new InjectionHandle());
    if (!injectionHandle.IsValid() || !injectionHandle->IsValid())
    {
        return;
    }

    this->nblPool.Update(new NetBufferListPool(DriverObject));
    if (!nblPool.IsValid() || !nblPool->IsValid())
    {
        return;
    }

    this->authConnectIpv4Callout.Update(new NetworkCallout{ this->deviceObject, this->engine,ClassifyFn, NotifyFn, FlowDeleteFn, FWPM_LAYER_ALE_AUTH_CONNECT_V4,gAuthConnectIpV4GUID, FWP_ACTION_CALLOUT_INSPECTION });
    if (!this->authConnectIpv4Callout.IsValid() || !this->authConnectIpv4Callout->IsValid())
    {
        return;
    }
    
    this->authRecvIpv4Callout.Update(new NetworkCallout{ this->deviceObject, this->engine,ClassifyFn, NotifyFn, FlowDeleteFn, FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V4, gAuthRecvAcceptIpv4GUID, FWP_ACTION_CALLOUT_INSPECTION });
    if (!this->authRecvIpv4Callout.IsValid() || !this->authRecvIpv4Callout->IsValid())
    {
        return;
    }
    
    this->authConnectIpv6Callout.Update(new NetworkCallout{ this->deviceObject, this->engine,ClassifyFn, NotifyFn, FlowDeleteFn, FWPM_LAYER_ALE_AUTH_CONNECT_V6, gAuthConnectIpV6GUID, FWP_ACTION_CALLOUT_INSPECTION });
    if (!this->authConnectIpv6Callout.IsValid() || !this->authConnectIpv6Callout->IsValid())
    {
        return;
    }
    
    this->authRecvIpv6Callout.Update(new NetworkCallout{ this->deviceObject, this->engine,ClassifyFn, NotifyFn, FlowDeleteFn, FWPM_LAYER_ALE_AUTH_RECV_ACCEPT_V6, gAuthRecvAcceptIpv6GUID, FWP_ACTION_CALLOUT_INSPECTION });
    if (!this->authRecvIpv6Callout.IsValid() || !this->authRecvIpv6Callout->IsValid())
    {
        return;
    }

    /// INLINE stream editing
    this->streamIpv4Callout.Update(new NetworkCallout{ this->deviceObject, this->engine,InlineStreamClassifyFn, NotifyFn, FlowDeleteFn, FWPM_LAYER_STREAM_V4, gStreamLayerIpV4GUID, FWP_ACTION_CALLOUT_TERMINATING });
    if (!this->streamIpv4Callout.IsValid() || !this->streamIpv4Callout->IsValid())
    {
        return;
    }

    this->streamIpv6Callout.Update(new NetworkCallout{ this->deviceObject, this->engine,InlineStreamClassifyFn, NotifyFn, FlowDeleteFn, FWPM_LAYER_STREAM_V6, gStreamLayerIpV6GUID, FWP_ACTION_CALLOUT_TERMINATING });
    if (!this->streamIpv6Callout.IsValid() || !this->streamIpv6Callout->IsValid())
    {
        return;
    }

    Validate();
}

Minifilter::NetworkFilter::~NetworkFilter()
{
    this->authConnectIpv4Callout.Update(nullptr);
    this->authRecvIpv4Callout.Update(nullptr);

    this->authConnectIpv6Callout.Update(nullptr);
    this->authRecvIpv6Callout.Update(nullptr);

    this->streamIpv4Callout.Update(nullptr);
    this->streamIpv6Callout.Update(nullptr);

    this->injectionHandle.Update(nullptr);
    this->nblPool.Update(nullptr);

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

    bool ok = GetNetworkTupleIndexesForLayer(
        FixedValues->layerId, 
        &AppIdIndex, 
        &LocalAddressIndex, 
        &RemoteAddressIndex, 
        &LocalPortIndex, 
        &RemotePortIndex, 
        &ProtocolIndex, 
        &IcmpIndex
    );

    if (ok)
    {
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

    Classify->actionType = FWP_ACTION_CONTINUE;
}

void 
Minifilter::NetworkFilter::InlineStreamClassifyFn(
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

    FWPS_STREAM_CALLOUT_IO_PACKET* ioPacket = (FWPS_STREAM_CALLOUT_IO_PACKET*)LayerData;
    FWPS_STREAM_DATA* streamData = ioPacket->streamData;
    BYTE* buffer = nullptr;

    if (streamData->dataLength == 0 || 
        FlagOn(streamData->flags,FWPS_STREAM_FLAG_SEND_DISCONNECT) ||
        FlagOn(streamData->flags, FWPS_STREAM_FLAG_RECEIVE_DISCONNECT))
    {
        PermitBytes(ioPacket, Classify, 0);
        return;
    }

    auto status = CopyStreamDataToBuffer(streamData, (PVOID*)&buffer);
    if (!NT_SUCCESS(status))
    {
        Classify->actionType = FWP_ACTION_CONTINUE;
        return;
    }

    ProcessDataStream(FixedValues, MetaValues, Filter, ioPacket, Classify, buffer, streamData->dataLength);
    ExFreePoolWithTag(buffer, STREAM_EDIT_TAG_BUFFER);
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
    case FWPS_LAYER_ALE_AUTH_CONNECT_V6:
        *AppIdIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_ALE_APP_ID;
        *LocalAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_LOCAL_ADDRESS;
        *RemoteAddressIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_REMOTE_ADDRESS;
        *LocalPortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_LOCAL_PORT;
        *RemotePortIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_REMOTE_PORT;
        *ProtocolIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_IP_PROTOCOL;
        *IcmpIndex = FWPS_FIELD_ALE_AUTH_CONNECT_V6_ICMP_TYPE;
        return true;
    case FWPS_LAYER_ALE_AUTH_RECV_ACCEPT_V6:
        *AppIdIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_ALE_APP_ID;
        *LocalAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_LOCAL_ADDRESS;
        *RemoteAddressIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_REMOTE_ADDRESS;
        *LocalPortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_LOCAL_PORT;
        *RemotePortIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_REMOTE_PORT;
        *ProtocolIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_IP_PROTOCOL;
        *IcmpIndex = FWPS_FIELD_ALE_AUTH_RECV_ACCEPT_V6_ICMP_TYPE;
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
Minifilter::NetworkFilter::ProcessIpValues(
    _In_ const FWP_VALUE0& AppId,
    _In_ const FWP_VALUE0& LocalAddress,
    _In_ const FWP_VALUE0& RemoteAddress,
    _In_ const FWP_VALUE0& LocalPort,
    _In_ const FWP_VALUE0& RemotePort,
    _In_ const FWP_VALUE0& Protocol,
    _In_ const FWP_VALUE0& Icmp,
    _In_ HANDLE ProcessId
)
{
    if (AppId.type != FWP_BYTE_BLOB_TYPE)
    {
        MyDriverLogError("Invalid type for application id");
        return;
    }

    if (LocalPort.type != FWP_UINT16 || LocalPort.type != FWP_UINT16)
    {
        MyDriverLogError("Invalid type for port Local = 0x%d Remote = 0x%d", LocalPort.type, RemotePort.type);
        return;
    }

    if (Protocol.type != FWP_UINT8)
    {
        MyDriverLogError("Invalid type for protocol type = %d", Protocol.type);
        return;
    }

    if (Icmp.type != FWP_UINT16)
    {
        MyDriverLogError("Invalid type for ICMP type = %d", Icmp.type);
        return;
    }

    if (LocalAddress.type == FWP_UINT32 && RemoteAddress.type == FWP_UINT32)
    {
        ProcessIpV4Values(AppId, LocalAddress, RemoteAddress, LocalPort, RemotePort, Protocol, Icmp, ProcessId);
    }
    else if (LocalAddress.type == FWP_BYTE_ARRAY16_TYPE && RemoteAddress.type == FWP_BYTE_ARRAY16_TYPE)
    {
        ProcessIpV6Values(AppId, LocalAddress, RemoteAddress, LocalPort, RemotePort, Protocol, Icmp, ProcessId);
    }
    else
    {
        MyDriverLogError("Invalid type for address Local = %d Remote = %d ", LocalAddress.type, RemoteAddress.type);
    }
}

void 
Minifilter::NetworkFilter::ProcessIpV4Values(
    _In_ const FWP_VALUE0& AppId,
    _In_ const FWP_VALUE0& LocalAddress,
    _In_ const FWP_VALUE0& RemoteAddress,
    _In_ const FWP_VALUE0& LocalPort,
    _In_ const FWP_VALUE0& RemotePort,
    _In_ const FWP_VALUE0& Protocol,
    _In_ const FWP_VALUE0& Icmp,
    _In_ HANDLE ProcessId
)
{
    UNICODE_STRING appIdBlob{ 0 };
    appIdBlob.Buffer = (PWCHAR)AppId.byteBlob->data;
    appIdBlob.Length = static_cast<USHORT>(AppId.byteBlob->size);
    appIdBlob.MaximumLength = static_cast<USHORT>(AppId.byteBlob->size);

    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);

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

    gDrvData.CommunicationPort->Send<KmUmShared::NetworkMessageIpV4>(
        ProcessId, 
        timestamp, 
        Cpp::NonPagedString{AppId.byteBlob->data, AppId.byteBlob->size },
        LocalAddress.uint32,
        RemoteAddress.uint32,
        LocalPort.uint16,
        RemotePort.uint16,
        Protocol.uint8,
        Icmp.uint16
    );
}

void
Minifilter::NetworkFilter::ProcessIpV6Values(
    _In_ const FWP_VALUE0& AppId,
    _In_ const FWP_VALUE0& LocalAddress,
    _In_ const FWP_VALUE0& RemoteAddress,
    _In_ const FWP_VALUE0& LocalPort,
    _In_ const FWP_VALUE0& RemotePort,
    _In_ const FWP_VALUE0& Protocol,
    _In_ const FWP_VALUE0& Icmp,
    _In_ HANDLE ProcessId
)
{
    UNICODE_STRING appIdBlob{ 0 };
    appIdBlob.Buffer = (PWCHAR)AppId.byteBlob->data;
    appIdBlob.Length = static_cast<USHORT>(AppId.byteBlob->size);
    appIdBlob.MaximumLength = static_cast<USHORT>(AppId.byteBlob->size);

    unsigned __int64 timestamp = 0;
    KeQuerySystemTime(&timestamp);

    MyDriverLogTrace("New data available: "
        "AppId = %wZ LocalAddress = %S RemoteAddress = %S LocalPort = 0x%x RemotePort = 0x%x, Protocol = 0x%x ICMP = 0x%x",
        &appIdBlob,
        LocalAddress.unicodeString,
        RemoteAddress.unicodeString,
        LocalPort.uint16,
        RemotePort.uint16,
        Protocol.uint8,
        Icmp.uint16
    );

    gDrvData.CommunicationPort->Send<KmUmShared::NetworkMessageIpV6>(
        ProcessId,
        timestamp,
        Cpp::NonPagedString{ AppId.byteBlob->data, AppId.byteBlob->size },
        Cpp::NonPagedString{ LocalAddress.byteArray16->byteArray16, sizeof(LocalAddress.byteArray16->byteArray16) },
        Cpp::NonPagedString{ RemoteAddress.byteArray16->byteArray16, sizeof(RemoteAddress.byteArray16->byteArray16) },
        LocalPort.uint16,
        RemotePort.uint16,
        Protocol.uint8,
        Icmp.uint16
    );
}

NTSTATUS 
Minifilter::NetworkFilter::CopyStreamDataToBuffer(
    _In_ FWPS_STREAM_DATA* StreamData,
    _Out_ PVOID* Buffer
)
{
    *Buffer = nullptr;

    SIZE_T bytesCopied = 0;
    auto buffer = ExAllocatePoolWithTag(NonPagedPool, StreamData->dataLength, STREAM_EDIT_TAG_BUFFER);
    if (!buffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ::FwpsCopyStreamDataToBuffer(StreamData, buffer, StreamData->dataLength, &bytesCopied);
    if (bytesCopied != StreamData->dataLength)
    {
        ExFreePoolWithTag(buffer, STREAM_EDIT_TAG_BUFFER);
        return STATUS_INVALID_BUFFER_SIZE;
    }

    *Buffer = buffer;
    return STATUS_SUCCESS;
}

void 
Minifilter::NetworkFilter::ProcessDataStream(
    _In_ const FWPS_INCOMING_VALUES0* FixedValues,
    _In_ const FWPS_INCOMING_METADATA_VALUES0* MetaValues,
    _In_ const FWPS_FILTER2* Filter,
    _Inout_ FWPS_STREAM_CALLOUT_IO_PACKET* IoPacket,
    _Inout_ FWPS_CLASSIFY_OUT* Classify,
    _In_ PVOID Buffer,
    _In_ SIZE_T BufferSize
)
{
    PBYTE buffer = (PBYTE)Buffer;

    PBYTE pattern = (PBYTE)gDrvData.NetworkStringToBeReplaced.GetNakedPointer();
    SIZE_T patternSize = gDrvData.NetworkStringToBeReplaced.GetSize();

    SIZE_T i = 0;

    LogBuffer(buffer, (USHORT)BufferSize);

    // Stream buffer begins with pattern so we replace it
    if (BufferSize >= patternSize && patternSize == RtlCompareMemory(pattern, buffer, patternSize))
    {
        MyDriverLogTrace("A new match was found!");

        auto status = InjectBuffer(
            gDrvData.NetworkFilter->injectionHandle,
            gDrvData.NetworkFilter->nblPool,
            gDrvData.NetworkStringToReplace,
            MetaValues->flowHandle,
            FixedValues->layerId,
            IoPacket->streamData->flags,
            Filter->action.calloutId
        );

        // If we successfuly injected a replacement buffer, we block the pattern, otherwise we allow it
        (NT_SUCCESS(status)) ? BlockBytes(IoPacket, Classify, patternSize)
                             : PermitBytes(IoPacket, Classify, patternSize);

        return;
    }

    // Skip bytes until we got a pattern match
    for (i = 0; i < BufferSize; ++i)
    {
        if (i + patternSize > BufferSize)
        {
            i = BufferSize;
            break;
        }

        if (RtlCompareMemory(&buffer[i], pattern, patternSize) == patternSize)
        {
            break;
        }
    }

    PermitBytes(IoPacket, Classify, i);
}

NTSTATUS 
Minifilter::NetworkFilter::InjectBuffer(
    _In_ Cpp::UniquePointer<InjectionHandle>& InjectionHandle,
    _In_ Cpp::UniquePointer<NetBufferListPool>& NblPool,
    _In_ Cpp::NonPagedString& ReplacementBuffer,
    _In_ UINT64 FlowHandle,
    _In_ UINT16 LayerId,
    _In_ UINT32 StreamFlags,
    _In_ UINT32 CalloutId
)
{
    NET_BUFFER_LIST* nbl = nullptr;

    auto status = ::FwpsAllocateNetBufferAndNetBufferList(
        NblPool->netBufferListPool,         // PoolHandle
        0,                                  // ContextSize
        0,                                  // ContextBackFill
        gDrvData.NetworkStringToReplaceMdl, // MdlChain
        0,                                  // DataOffset
        ReplacementBuffer.GetSize(),        // DataLength
        &nbl                                // NetBufferList
    );

    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::FwpsAllocateNetBufferAndNetBufferList has failed with status 0x%x", status);
        return status;
    }

    status = ::FwpsStreamInjectAsync(
        InjectionHandle->injectionHandle,   // InjectionHandle
        nullptr,                            // InjectionContext
        0,                                  // Flags
        FlowHandle,                         // FlowId
        CalloutId,                          // CalloutId
        LayerId,                            // LayerId
        StreamFlags,                        // StreamFlags
        nbl,                                // NetBufferList
        ReplacementBuffer.GetSize(),        // DataLength
        InjectCompletionRoutine,            // CompletionFn
        nullptr                             // CompletionContext
    );

    if (!NT_SUCCESS(status))
    {
        ::FwpsFreeNetBufferList(nbl);

        MyDriverLogError("::FwpsStreamInjectAsync has failed with status 0x%x", status);
        return status;
    }

    return STATUS_SUCCESS;
}

void 
Minifilter::NetworkFilter::PermitBytes(
    _Inout_ FWPS_STREAM_CALLOUT_IO_PACKET* IoPacket,
    _Inout_ FWPS_CLASSIFY_OUT* Classify,
    _In_ SIZE_T Bytes
)
{
    IoPacket->streamAction = FWPS_STREAM_ACTION_NONE; 
    IoPacket->countBytesEnforced = Bytes; 
    Classify->actionType = FWP_ACTION_PERMIT; 
}

void 
Minifilter::NetworkFilter::BlockBytes(
    _Inout_ FWPS_STREAM_CALLOUT_IO_PACKET* IoPacket,
    _Inout_ FWPS_CLASSIFY_OUT* Classify,
    _In_ SIZE_T Bytes
)
{
    IoPacket->streamAction = FWPS_STREAM_ACTION_NONE;
    IoPacket->countBytesEnforced = Bytes;
    Classify->actionType = FWP_ACTION_BLOCK;
}

void 
Minifilter::NetworkFilter::InjectCompletionRoutine(
    _In_ void* Context,
    _Inout_ NET_BUFFER_LIST* NetBufferList,
    _In_ BOOLEAN DispatchLevel
)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DispatchLevel);

    ::FwpsFreeNetBufferList(NetBufferList);
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

Minifilter::NetworkCallout::NetworkCallout(
    Cpp::UniquePointer<DeviceObject>& DeviceObject,
    Cpp::SharedPointer<NetworkEngine>& Engine,
    FWPS_CALLOUT_CLASSIFY_FN2 ClassifyFunction,
    FWPS_CALLOUT_NOTIFY_FN2 NotifyFunction,
    FWPS_CALLOUT_FLOW_DELETE_NOTIFY_FN0 FlowDeleteFunction,
    const GUID& LayerKey,
    const GUID& CalloutKey,
    const FWP_ACTION_TYPE& ActionType
) : engine{Engine},
    classifyFunction{ClassifyFunction},
    notifyFunction{NotifyFunction},
    flowDeleteFunction{FlowDeleteFunction}
{
    if (!engine.IsValid() || !engine->IsValid())
    {
        return;
    }

    RtlCopyMemory(&this->layerKey, &LayerKey, sizeof(GUID));
    RtlCopyMemory(&this->calloutKey, &CalloutKey, sizeof(GUID));

    if (RegisterCallout(DeviceObject, ActionType))
    {
        Validate();
    }
}

Minifilter::NetworkCallout::~NetworkCallout()
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    if (this->IsValid())
    {
        status = ::FwpsCalloutUnregisterById(this->calloutFwpsId);
        NT_VERIFY(NT_SUCCESS(status));

        status = ::FwpmFilterDeleteById(this->engine->engineHandle, this->calloutFilterId);
        NT_VERIFY(NT_SUCCESS(status));

        status = ::FwpmCalloutDeleteById(this->engine->engineHandle, this->calloutFwpmId);
        NT_VERIFY(NT_SUCCESS(status));
    }

    this->engine.Update(nullptr);
}

NTSTATUS 
Minifilter::NetworkCallout::RegisterFwpsCallout(
    Cpp::UniquePointer<DeviceObject>& DeviceObject
)
{
    FWPS_CALLOUT fwpsCallout = { 0 };

    fwpsCallout.calloutKey = this->calloutKey;
    fwpsCallout.classifyFn = this->classifyFunction;
    fwpsCallout.notifyFn = this->notifyFunction;
    fwpsCallout.flowDeleteFn = this->flowDeleteFunction;

    return ::FwpsCalloutRegister(DeviceObject->GetDeviceObject(), &fwpsCallout, &this->calloutFwpsId);
}

NTSTATUS 
Minifilter::NetworkCallout::RegisterFwpmCallout(
    Cpp::UniquePointer<DeviceObject>& DeviceObject
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    FWPM_CALLOUT fwpmCallout = { 0 };
    FWPM_DISPLAY_DATA displayData = { 0 };

    displayData.name = L"Dummy Name";
    displayData.description = L"Dummy Description";

    fwpmCallout.calloutKey = this->calloutKey;
    fwpmCallout.displayData = displayData;
    fwpmCallout.applicableLayer = this->layerKey;

    return ::FwpmCalloutAdd(this->engine->engineHandle, &fwpmCallout, nullptr, &this->calloutFwpmId);
}

NTSTATUS 
Minifilter::NetworkCallout::RegisterFilter(
    Cpp::UniquePointer<DeviceObject>& DeviceObject,
    const FWP_ACTION_TYPE& ActionType
)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    FWPM_FILTER filter = { 0 };

    filter.layerKey = this->layerKey;
    filter.displayData.name = L"Dummy filter name";
    filter.displayData.description = L"Dummy Description";

    filter.action.type = ActionType;
    filter.action.calloutKey = this->calloutKey;
    filter.weight.type = FWP_EMPTY;

    return FwpmFilterAdd(this->engine->engineHandle, &filter, nullptr, &this->calloutFilterId);
}

bool 
Minifilter::NetworkCallout::RegisterCallout(
    Cpp::UniquePointer<DeviceObject>& DeviceObject,
    const FWP_ACTION_TYPE& ActionType
)
{
    auto status = RegisterFwpsCallout(DeviceObject);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("RegisterFwpsCallout failed with status 0x%x", status);
        return false;
    }

    status = RegisterFwpmCallout(DeviceObject);
    if (!NT_SUCCESS(status))
    {
        FwpsCalloutUnregisterById(this->calloutFwpsId);

        MyDriverLogCritical("RegisterFwpmCallout failed with status 0x%x", status);
        return false;
    }

    status = RegisterFilter(DeviceObject, ActionType);
    if (!NT_SUCCESS(status))
    {
        FwpsCalloutUnregisterById(this->calloutFwpsId);
        FwpmCalloutDeleteById(this->engine->engineHandle, this->calloutFwpmId);

        MyDriverLogCritical("RegisterFilter for callout failed with status 0x%x", status);
        return false;
    }

    return true;
}

Minifilter::NetBufferListPool::NetBufferListPool(PDRIVER_OBJECT DriverObject)
{
    NET_BUFFER_LIST_POOL_PARAMETERS nblPoolParams = { 0 };

    this->ndisGenericObject = ::NdisAllocateGenericObject(DriverObject, STREAM_EDIT_TAG_NBL_POOL, 0);
    if (!this->ndisGenericObject)
    {
        MyDriverLogCritical("::NdisAllocateGenericObject failed!");
        return;
    }

    nblPoolParams.Header.Type = NDIS_OBJECT_TYPE_DEFAULT;
    nblPoolParams.Header.Revision = NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    nblPoolParams.Header.Size = NDIS_SIZEOF_NET_BUFFER_LIST_POOL_PARAMETERS_REVISION_1;
    nblPoolParams.fAllocateNetBuffer = TRUE;
    nblPoolParams.DataSize = 0;
    nblPoolParams.PoolTag = STREAM_EDIT_TAG_NBL_POOL;

    this->netBufferListPool = ::NdisAllocateNetBufferListPool(this->ndisGenericObject, &nblPoolParams);
    if (!this->netBufferListPool)
    {
        MyDriverLogCritical("::NdisAllocateNetBufferListPool failed!");
        return;
    }

    this->Validate();
}

Minifilter::NetBufferListPool::~NetBufferListPool()
{
    if (this->netBufferListPool)
    {
        ::NdisFreeNetBufferListPool(this->netBufferListPool);
        this->netBufferListPool = nullptr;
    }

    if (this->ndisGenericObject)
    {
        ::NdisFreeGenericObject(this->ndisGenericObject);
        this->ndisGenericObject = nullptr;
    }
}

Minifilter::InjectionHandle::InjectionHandle()
{
    auto status = ::FwpsInjectionHandleCreate(AF_UNSPEC, FWPS_INJECTION_TYPE_STREAM, &this->injectionHandle);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogCritical("::FwpsInjectionHandleCreate failed with status 0x%X!", status);
        return;
    }

    Validate();
}

Minifilter::InjectionHandle::~InjectionHandle()
{
    if (this->injectionHandle)
    {
        auto status = ::FwpsInjectionHandleDestroy(this->injectionHandle);
        NT_VERIFY(NT_SUCCESS(status));
    }
}
