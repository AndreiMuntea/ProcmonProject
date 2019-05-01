#include "FltPort.hpp"
#include "trace.hpp"
#include "FltPort.tmh"

#include <cpp_lockguard.hpp>
#include <CppSemantics.hpp>


Minifilter::FltPort::FltPort(
    _In_ PFLT_FILTER Filter,
    _In_ PUNICODE_STRING PortName,
    _In_ PFUNC_OnMessageNotifyCallback OnMessageNotify
) : filter{ Filter },
    threadpool{10},
    onMessageNotify{OnMessageNotify}
{
    OBJECT_ATTRIBUTES objAttr = { 0 };
    PSECURITY_DESCRIPTOR securityDescriptor = nullptr;

    if (!threadpool.IsValid())
    {
        MyDriverLogError("Couldn't start threadpool");
        return;
    }

    auto status = ::FltBuildDefaultSecurityDescriptor(&securityDescriptor, FLT_PORT_ALL_ACCESS);
    if (!NT_VERIFY(NT_SUCCESS(status)))
    {
        MyDriverLogError("::FltBuildDefaultSecurityDescriptor has failed with status 0x%x", status);
        Invalidate();
        return;
    }

    InitializeObjectAttributes(&objAttr, PortName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, securityDescriptor);

    status = ::FltCreateCommunicationPort(
        Filter,                                 // Filter   
        &this->serverPort,                      // ServerPort
        &objAttr,                               // ObjectAttributes
        this,                                   // ServerPortCookie
        &FltPort::ConnectNotifyCallback,        // ConnectNotifyCallback
        &FltPort::DisconnectNotifyCallback,     // DisconnectNotifyCallback
        &FltPort::MessageNotifyCallback,        // MessageNotifyCallback
        1                                       // MaxConnections
    );

    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::FltCreateCommunicationPort has failed with status 0x%x", status);
        ::FltFreeSecurityDescriptor(securityDescriptor);
        Invalidate();
        return;
    }

    MyDriverLogTrace("Created server communication serverPort %wZ", PortName);
    ::FltFreeSecurityDescriptor(securityDescriptor);
    Validate();
}

NTSTATUS 
Minifilter::FltPort::Send(Cpp::Stream&& DataStream)
{
    FltPortDataPackage* package = new FltPortDataPackage(this, Cpp::Forward<Cpp::Stream>(DataStream));
    if (!package)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    if (!package->IsValid())
    {
        delete package;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    auto status = threadpool.EnqueueItem(DataPackageCallback, DataPackageCleanupCallback, package);
    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("EnqueueItem failed with status 0x%x", status);
        delete package;
    }

    return status;
}

Minifilter::FltPort::~FltPort()
{
    if (threadpool.IsValid())
    {
        threadpool.Shutdown();
    }

    if (!this->IsValid())
    {
        return;
    }

    Cpp::ExclusiveLockguard guard(&this->lock);

    CloseClientPort();
    CloseServerPort();
}

void 
Minifilter::FltPort::DataPackageCleanupCallback(
    PVOID Context
)
{
    auto dataPackage = (FltPortDataPackage*)(Context);
    delete dataPackage;
}

void 
Minifilter::FltPort::DataPackageCallback(
    PVOID Context
)
{
    auto dataPackage = (FltPortDataPackage*)(Context);

    LARGE_INTEGER timeout = { 0 };
    timeout.QuadPart = -60 * 10 * 1000 * 1000; // 60 seconds

    NTSTATUS replyStatus = STATUS_UNSUCCESSFUL;
    ULONG replySize = sizeof(replyStatus);

    Cpp::SharedLockguard guard(&dataPackage->port->lock);
    if (!dataPackage->port->clientPort)
    {
        MyDriverLogWarning("Client is disconnected. Cannot send message");
        goto CleanUp;
    }

    if (!dataPackage->dataStream.IsValid() || !dataPackage->dataStream.GetRawData())
    {
        MyDriverLogError("Buffer is not valid");
        goto CleanUp;
    }

    if (dataPackage->dataStream.GetSize() > MAXULONG || dataPackage->dataStream.GetSize() == 0)
    {
        MyDriverLogError("Buffer size is not valid");
        goto CleanUp;
    }

    auto status = ::FltSendMessage(
        dataPackage->port->filter,
        &dataPackage->port->clientPort,
        dataPackage->dataStream.GetRawData(),
        static_cast<ULONG>(dataPackage->dataStream.GetSize()),
        &replyStatus,
        &replySize,
        &timeout
    );

    if (!NT_SUCCESS(status))
    {
        MyDriverLogError("::FltSendMessage failed with status 0x%X", status);
        goto CleanUp;
    }

    if (status == STATUS_TIMEOUT)
    {
        MyDriverLogError("::FltSendMessage timeout");
        goto CleanUp;
    }

    if (replySize != sizeof(replyStatus))
    {
        MyDriverLogError("::FltSendMessage invalid reply size");
        goto CleanUp;
    }

    MyDriverLogTrace("::FltSendMessage sent message");

CleanUp:
    delete dataPackage;
}

void 
Minifilter::FltPort::CloseClientPort()
{
    if (this->clientPort)
    {
        MyDriverLogTrace("Closing client communication port");
        ::FltCloseClientPort(this->filter, &this->clientPort);
        this->clientPort = nullptr;
    }
}

void 
Minifilter::FltPort::CloseServerPort()
{
    if (this->serverPort)
    {
        MyDriverLogTrace("Closing server communication port");
        ::FltCloseCommunicationPort(this->serverPort);
        this->serverPort = nullptr;
    }
}

NTSTATUS FLTAPI
Minifilter::FltPort::ConnectNotifyCallback(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
)
{
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);

    MyDriverLogTrace("We are in ConnectNotifyCallback");

    FltPort* port = static_cast<FltPort*>(ServerPortCookie);
    if (!port)
    {
        return STATUS_INVALID_PARAMETER;
    }

    Cpp::ExclusiveLockguard guard(&port->lock);
    if (port->clientPort)
    {
        MyDriverLogWarning("A client is already connected");
        return STATUS_ALREADY_REGISTERED;
    }

    port->clientPort = ClientPort;
    *ConnectionPortCookie = port;

    return STATUS_SUCCESS;
}

void FLTAPI
Minifilter::FltPort::DisconnectNotifyCallback(
    _In_opt_ PVOID ConnectionCookie
)
{
    MyDriverLogTrace("We are in DisconnectNotifyCallback");

    FltPort* fltPort = static_cast<FltPort*>(ConnectionCookie);
    if (fltPort)
    {
        Cpp::ExclusiveLockguard guard(&fltPort->lock);
        fltPort->CloseClientPort();
    }
}

NTSTATUS FLTAPI 
Minifilter::FltPort::MessageNotifyCallback(
    _In_opt_ PVOID PortCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
)
{
    MyDriverLogTrace("We are in MessageNotifyCallback");

    FltPort* fltPort = static_cast<FltPort*>(PortCookie);
    if (fltPort)
    {
        Cpp::ExclusiveLockguard guard(&fltPort->lock);
        return fltPort->onMessageNotify(InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, ReturnOutputBufferLength);
    }

    return STATUS_UNSUCCESSFUL;
}

Minifilter::FltPortDataPackage::FltPortDataPackage(
    FltPort* Port,
    Cpp::Stream&& DataStream
) : port{Port},
    dataStream{Cpp::Forward<Cpp::Stream>(DataStream)}
{
    Validate();
}

Minifilter::FltPortDataPackage::~FltPortDataPackage()
{
    port = nullptr;
}
