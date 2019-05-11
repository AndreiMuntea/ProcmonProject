#ifndef __FLT_PORT_HPP__ 
#define __FLT_PORT_HPP__

#include <CppStream.hpp>
#include <CppString.hpp>
#include <cpp_lockguard.hpp>

#include "cpp_allocator_object.hpp"
#include "cpp_lock.hpp"
#include "ThreadPool.hpp"
#include "ProcessCollector.hpp"

namespace Minifilter
{
    typedef NTSTATUS FUNC_OnMessageNotifyCallback(
        _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
        _In_ ULONG InputBufferLength,
        _Out_writes_bytes_to_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
        _In_ ULONG OutputBufferLength,
        _Out_ PULONG ReturnOutputBufferLength
    );
    typedef FUNC_OnMessageNotifyCallback *PFUNC_OnMessageNotifyCallback;

    template <class MessageType>
    class FltPortDataPackage : public Cpp::CppNonPagedObject<'TLF#'>
    {
        friend class FltPort;
    public:
        FltPortDataPackage(
            FltPort* Port,
            HANDLE ProcessId,
            unsigned __int64 Timestamp,
            MessageType Message
        );

        virtual ~FltPortDataPackage() = default;

    private:
        FltPort* port = nullptr;            
        HANDLE processId;
        unsigned __int64 timestamp;
        MessageType message;
    };

    class FltPort : public Cpp::CppNonPagedObject<'TLF#'>
    {
    public:
        FltPort(
            _In_ PFLT_FILTER Filter,
            _In_ PUNICODE_STRING PortName,
            _In_ PFUNC_OnMessageNotifyCallback OnMessageNotify,
            _In_ ProcessCollector* ProcessCollector
        );

        FltPort(const FltPort& Other) = delete;
        FltPort(FltPort&& Other) = delete;

        FltPort& operator=(const FltPort& Other) = delete;
        FltPort& operator=(FltPort&& Other) = delete;

        template <class MessageType, class ...Args>
        NTSTATUS 
        Send(
            _In_ HANDLE ProcessId,
            _In_ unsigned __int64 Timestamp,
            Args... Arguments
        );

        virtual ~FltPort();

    private:

        template <class MessageType>
        static void DataPackageCleanupCallback(PVOID Context);

        template <class MessageType>
        static void DataPackageCallback(PVOID Context);

        static void SendStreamMessage(
            FltPort* Port,
            Cpp::Stream& Stream
        );

        PFUNC_OnMessageNotifyCallback onMessageNotify = nullptr;
        PFLT_FILTER filter = nullptr;
        PFLT_PORT clientPort = nullptr;
        PFLT_PORT serverPort = nullptr;
        ProcessCollector* processCollector = nullptr;
        Cpp::Pushlock lock;
        Cpp::ThreadPool threadpool;

        void CloseClientPort();
        void CloseServerPort();

        static NTSTATUS FLTAPI
        ConnectNotifyCallback(
            _In_ PFLT_PORT ClientPort,
            _In_opt_ PVOID ServerPortCookie,
            _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
            _In_ ULONG SizeOfContext,
            _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
        );

        static void FLTAPI
        DisconnectNotifyCallback(
            _In_opt_ PVOID ConnectionCookie
        );

        static NTSTATUS FLTAPI
        MessageNotifyCallback(
            _In_opt_ PVOID PortCookie,
            _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
            _In_ ULONG InputBufferLength,
            _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
            _In_ ULONG OutputBufferLength,
            _Out_ PULONG ReturnOutputBufferLength
        );
    };

    template<class MessageType>
    inline FltPortDataPackage<MessageType>::FltPortDataPackage(
        FltPort * Port, 
        HANDLE ProcessId, 
        unsigned __int64 Timestamp, 
        MessageType Message
    ) : port{Port},
        processId{ProcessId},
        timestamp{Timestamp},
        message{Message}
    {
        Validate();
    }
    template<class MessageType, class ...Args>
    inline NTSTATUS FltPort::Send(HANDLE ProcessId, unsigned __int64 Timestamp, Args ...Arguments)
    {
        auto package = new FltPortDataPackage<MessageType>(this, ProcessId, Timestamp, MessageType{ Arguments... });
        if (!package)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        if (!package->IsValid())
        {
            delete package;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        auto status = threadpool.EnqueueItem(DataPackageCallback<MessageType>, DataPackageCleanupCallback<MessageType>, package);
        if (!NT_SUCCESS(status))
        {
            delete package;
        }

        return status;
    }
    template<class MessageType>
    inline void FltPort::DataPackageCleanupCallback(PVOID Context)
    {
        auto dataPackage = (FltPortDataPackage<MessageType>*)(Context);
        delete dataPackage;
    }
    template<class MessageType>
    inline void FltPort::DataPackageCallback(PVOID Context)
    {
        auto dataPackage = (FltPortDataPackage<MessageType>*)(Context);
        Cpp::String processName;
        Cpp::Stream stream;

        Cpp::SharedLockguard guard(&dataPackage->port->lock);
        if (!dataPackage->port->clientPort)
        {
            goto CleanUp;
        }

        if (!dataPackage->port->processCollector->GetProcessName(dataPackage->processId, dataPackage->timestamp, processName))
        {
            goto CleanUp;
        }

        stream << dataPackage->processId << processName << dataPackage->timestamp << dataPackage->message;
        SendStreamMessage(dataPackage->port, stream);

    CleanUp:
        delete dataPackage;
    }
}

#endif //__FLT_PORT_HPP__