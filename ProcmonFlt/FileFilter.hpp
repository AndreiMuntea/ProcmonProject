#ifndef __FILE_FILTER_HPP__ 
#define __FILE_FILTER_HPP__

#include "cpp_allocator_object.hpp"
#include <CppString.hpp>
#include <CppSemantics.hpp>

#include "../Common/FltPortFileMessage.hpp"
#include "ConfigurationManager.hpp"

namespace Minifilter
{
    typedef struct _FILE_STREAM_CONTEXT
    {
        UNICODE_STRING FileName;
    }FILE_STREAM_CONTEXT, *PFILE_STREAM_CONTEXT;

    class FileFilter : public Cpp::CppNonPagedObject<'TFF#'>
    {
    public: 
        FileFilter() = default;
        virtual ~FileFilter() = default;

        static FLT_PREOP_CALLBACK_STATUS FLTAPI
        PreCreateCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Outptr_result_maybenull_ PVOID *CompletionContext
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostCreateCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_PREOP_CALLBACK_STATUS FLTAPI
        PreCloseCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Outptr_result_maybenull_ PVOID *CompletionContext
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostCloseCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_PREOP_CALLBACK_STATUS FLTAPI
        PreCleanupCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Outptr_result_maybenull_ PVOID *CompletionContext
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostCleanupCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostCleanupSafeCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_PREOP_CALLBACK_STATUS FLTAPI
        PreReadCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Outptr_result_maybenull_ PVOID *CompletionContext
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostReadCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostReadSafeCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_PREOP_CALLBACK_STATUS FLTAPI
        PreWriteCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Outptr_result_maybenull_ PVOID *CompletionContext
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostWriteCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostWriteSafeCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_PREOP_CALLBACK_STATUS FLTAPI
        PreSetInformationCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Outptr_result_maybenull_ PVOID *CompletionContext
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostSetInformationCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static FLT_POSTOP_CALLBACK_STATUS FLTAPI
        PostSetInformationSafeCallback(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _In_opt_ PVOID CompletionContext,
            _In_ FLT_POST_OPERATION_FLAGS Flags
        );

        static void FLTAPI
        FileContextCleanup(
            _In_ PFLT_CONTEXT Context,
            _In_ FLT_CONTEXT_TYPE ContextType
        );

    private:   
        static bool 
        IsActionMonitored(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ const KmUmShared::Feature& FeatureToCheck
        );

        static bool 
        IsPathBlacklisted(
            _In_ UNICODE_STRING* FileName
        );

        static NTSTATUS 
        GetFileName(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Inout_ UNICODE_STRING& FileName
        );

        static void
        GetDirectoryName(
            _In_ UNICODE_STRING* FileName,
            _Inout_ UNICODE_STRING* DirectoryName
        );

        static NTSTATUS
        CreateStreamContext(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Out_ FILE_STREAM_CONTEXT** StreamContext
        );
        static NTSTATUS
        GetSetStreamContext(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            _Out_ FILE_STREAM_CONTEXT** StreamContext
        );

        template <class Message, class ...Args>
        static void NotifyEvent(
            _Inout_ PFLT_CALLBACK_DATA Data,
            _In_ PCFLT_RELATED_OBJECTS FltObjects,
            Args... Arguments
        );
    };

    template<class Message, class ...Args>
    inline void FileFilter::NotifyEvent(
        _Inout_ PFLT_CALLBACK_DATA Data,
        _In_ PCFLT_RELATED_OBJECTS FltObjects,
        Args... Arguments
    )
    {
        FILE_STREAM_CONTEXT* context = nullptr;
        Cpp::String file;

        unsigned __int64 timestamp = 0;
        KeQuerySystemTime(&timestamp);

        auto status = GetSetStreamContext(Data, FltObjects, &context);
        if (!NT_SUCCESS(status))
        {
            return;
        }
        if (!context || !context->FileName.Buffer)
        {
            goto CleanUp;
        }

        file = Cpp::String{ (const unsigned __int8*)context->FileName.Buffer, context->FileName.Length };
        gDrvData.CommunicationPort->Send<Message>(FltGetRequestorProcessIdEx(Data), timestamp, file, Data->IoStatus.Status, Arguments...);

    CleanUp:
        FltReleaseContext(context);
    }
}

#endif //__FILE_FILTER_HPP__