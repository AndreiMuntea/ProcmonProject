#ifndef __FLT_PORT_HPP__ 
#define __FLT_PORT_HPP__

#include <CppStream.hpp>

#include "cpp_allocator_object.hpp"
#include "cpp_lock.hpp"

namespace Minifilter
{

    class FltPort : public Cpp::CppNonPagedObject<'TLF#'>
    {
    public:
        FltPort(
            _In_ PFLT_FILTER Filter,
            _In_ PUNICODE_STRING PortName
        );

        FltPort(const FltPort& Other) = delete;
        FltPort(FltPort&& Other) = delete;

        FltPort& operator=(const FltPort& Other) = delete;
        FltPort& operator=(FltPort&& Other) = delete;

        NTSTATUS Send(Cpp::Stream& DataStream);

        virtual ~FltPort();

    private:
        PFLT_FILTER filter = nullptr;
        PFLT_PORT clientPort = nullptr;
        PFLT_PORT serverPort = nullptr;
        Cpp::Pushlock lock;

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
}

#endif //__FLT_PORT_HPP__