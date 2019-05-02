#include "FltPort.hpp"
#include "trace.hpp"
#include "FltPort.tmh"

#include <cpp_lockguard.hpp>
#include <CppSemantics.hpp>


Minifilter::FltPort::FltPort(
    _In_ PFLT_FILTER Filter,
    _In_ PUNICODE_STRING PortName,
    _In_ PFUNC_OnMessageNotifyCallback OnMessageNotify,
    _In_ ProcessCollector* ProcessCollector
) : filter{ Filter },
    threadpool{10},
    onMessageNotify{OnMessageNotify},
    processCollector{ProcessCollector}
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



void Minifilter::FltPort::SendStreamMessage(FltPort *Port, Cpp::Stream & Stream)
{
    NTSTATUS replyStatus = STATUS_UNSUCCESSFUL;
    ULONG replySize = sizeof(replyStatus);
    LARGE_INTEGER timeout = { 0 };

    timeout.QuadPart = -60 * 10 * 1000 * 1000; // 60 seconds

    if (!Stream.IsValid() || !Stream.GetRawData() || Stream.GetSize() == 0 || Stream.GetSize() > MAXULONG)
    {
        MyDriverLogError("Stream is not valid");
        return;
    }

    auto status = ::FltSendMessage(
        Port->filter,
        &Port->clientPort,
        Stream.GetRawData(),
        static_cast<ULONG>(Stream.GetSize()),
        &replyStatus,
        &replySize,
        &timeout
    );

    if (status == STATUS_TIMEOUT)
    {
        MyDriverLogError("::FltSendMessage timeout");
        return;
    }

    if (replySize != sizeof(replyStatus))
    {
        MyDriverLogError("::FltSendMessage invalid reply size");
        return;
    }
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
    
    if (!OutputBuffer || !InputBuffer)
    {
        return STATUS_INVALID_PARAMETER;
    }

    FltPort* fltPort = static_cast<FltPort*>(PortCookie);
    if (fltPort)
    {
        Cpp::ExclusiveLockguard guard(&fltPort->lock);
        return fltPort->onMessageNotify(InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, ReturnOutputBufferLength);
    }

    return STATUS_INVALID_PARAMETER;
}
