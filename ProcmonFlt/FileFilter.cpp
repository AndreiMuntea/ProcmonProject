#include "FileFilter.hpp"
#include "trace.hpp"
#include "FileFilter.tmh"

#include "GlobalData.hpp"
#include "ProcessUtils.hpp"

FLT_PREOP_CALLBACK_STATUS FLTAPI 
Minifilter::FileFilter::PreCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    return IsActionMonitored(Data, KmUmShared::Feature::featureMonitorFileCreate)   ? FLT_PREOP_SUCCESS_WITH_CALLBACK
                                                                                    : FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(CompletionContext);
    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
    {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    NotifyEvent<KmUmShared::FileCreateMessage>(Data, FltObjects);    
    
    // If file was opened with DELETE_ON_CLOSE we also notify delete operation
    if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DELETE_ON_CLOSE))
    {
        NotifyEvent<KmUmShared::FileDeleteMessage>(Data, FltObjects);
    }

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PreCloseCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (!IsActionMonitored(Data, KmUmShared::Feature::featureMonitorFileClose))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    NotifyEvent<KmUmShared::FileCloseMessage>(Data, FltObjects);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostCloseCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    UNREFERENCED_PARAMETER(Flags);

    /// SHOULDN'T BE CALLED
    NT_ASSERT(false);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PreCleanupCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (!IsActionMonitored(Data, KmUmShared::Feature::featureMonitorFileCleanup))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FILE_STREAM_CONTEXT* context = nullptr;
    auto status = GetSetStreamContext(Data, FltObjects, &context);
    if (!NT_SUCCESS(status))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FltReleaseContext(context);
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostCleanupCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(CompletionContext);
    FLT_POSTOP_CALLBACK_STATUS callbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
    {
        return callbackStatus;
    }

    FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, PostCleanupSafeCallback, &callbackStatus);
    return callbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostCleanupSafeCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NotifyEvent<KmUmShared::FileCleanupMessage>(Data, FltObjects);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

void 
Minifilter::FileFilter::FileContextCleanup(
    _In_ PFLT_CONTEXT Context,
    _In_ FLT_CONTEXT_TYPE ContextType
)
{
    PFILE_STREAM_CONTEXT context = (PFILE_STREAM_CONTEXT)(Context);
    if (ContextType == FLT_STREAM_CONTEXT && context->FileName.Buffer)
    {
        ExFreePoolWithTag(context->FileName.Buffer, 'TFF#');
        context->FileName = { 0,0, nullptr };
    }
}

bool
Minifilter::FileFilter::IsActionMonitored(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ const KmUmShared::Feature& FeatureToCheck
)
{
    auto processId = FltGetRequestorProcessIdEx(Data);

    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(KmUmShared::Feature::featureMonitorStarted))
    {
        return false;
    }

    if (!gDrvData.ConfigurationManager->IsFeatureEnabled(FeatureToCheck))
    {
        return false;
    }

    if (ProcessUtils::IsSystemOrIdleProcess(processId))
    {
        return false;
    }

    if (Data->RequestorMode == KernelMode)
    {
        return false;
    }

    return true;
}

NTSTATUS
Minifilter::FileFilter::GetFileName(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Inout_ UNICODE_STRING& FileName
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    PFLT_FILE_NAME_INFORMATION fileNameInfo = nullptr;

    auto status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &fileNameInfo);
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (!fileNameInfo->Name.Buffer || fileNameInfo->Name.Length == 0)
    {
        FltReleaseFileNameInformation(fileNameInfo);
        return STATUS_INVALID_BUFFER_SIZE;
    }

    FileName.Buffer = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool, fileNameInfo->Name.Length, 'TFF#');
    if (!FileName.Buffer)
    {
        FltReleaseFileNameInformation(fileNameInfo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(FileName.Buffer, fileNameInfo->Name.Buffer, fileNameInfo->Name.Length);
    FileName.Length = fileNameInfo->Name.Length;
    FileName.MaximumLength = fileNameInfo->Name.Length;
    
    FltReleaseFileNameInformation(fileNameInfo);

    return STATUS_SUCCESS;
}

NTSTATUS
Minifilter::FileFilter::CreateStreamContext(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ FILE_STREAM_CONTEXT** StreamContext
)
{
    *StreamContext = nullptr;
    FILE_STREAM_CONTEXT* context = nullptr;

    auto status = FltAllocateContext(FltObjects->Filter, FLT_STREAM_CONTEXT, sizeof(FILE_STREAM_CONTEXT), NonPagedPool, (PFLT_CONTEXT *)&context);
    if (!NT_SUCCESS(status))
    {
        return status;
    }
    RtlZeroMemory(context, sizeof(*context));

    status = GetFileName(Data, FltObjects, context->FileName);
    if (!NT_SUCCESS(status) || !context->FileName.Buffer)
    {
        FltReleaseContext(context);
        return STATUS_FILE_NOT_AVAILABLE;
    }

    status = FltSetStreamContext(FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, nullptr);
    if (status == STATUS_FLT_CONTEXT_ALREADY_DEFINED)
    {
        status = STATUS_SUCCESS;
    }

    FltReleaseContext(context);
    return status;
}

NTSTATUS
Minifilter::FileFilter::GetSetStreamContext(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ FILE_STREAM_CONTEXT** StreamContext
)
{
    *StreamContext = nullptr;

    auto status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, (PFLT_CONTEXT*)StreamContext);
    if (!NT_SUCCESS(status) && status != STATUS_NOT_FOUND)
    {
        return status;
    }

    if (status == STATUS_NOT_FOUND)
    {
        status = CreateStreamContext(Data, FltObjects, StreamContext);
        if (NT_SUCCESS(status))
        {
            status = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, (PFLT_CONTEXT*)StreamContext);
        }
    }
    
    return status;
}

FLT_PREOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PreReadCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (!IsActionMonitored(Data, KmUmShared::Feature::featureMonitorFileRead))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FILE_STREAM_CONTEXT* context = nullptr;
    auto status = GetSetStreamContext(Data, FltObjects, &context);
    if (!NT_SUCCESS(status))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FltReleaseContext(context);
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostReadCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(CompletionContext);
    FLT_POSTOP_CALLBACK_STATUS callbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
    {
        return callbackStatus;
    }

    FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, PostReadSafeCallback, &callbackStatus);
    return callbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostReadSafeCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);

    NotifyEvent<KmUmShared::FileReadMessage>(Data, FltObjects);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PreWriteCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (!IsActionMonitored(Data, KmUmShared::Feature::featureMonitorFileWrite))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FILE_STREAM_CONTEXT* context = nullptr;
    auto status = GetSetStreamContext(Data, FltObjects, &context);
    if (!NT_SUCCESS(status))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FltReleaseContext(context);
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostWriteCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(CompletionContext);
    FLT_POSTOP_CALLBACK_STATUS callbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
    {
        return callbackStatus;
    }

    FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, PostWriteSafeCallback, &callbackStatus);
    return callbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostWriteSafeCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);

    NotifyEvent<KmUmShared::FileWriteMessage>(Data, FltObjects);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

FLT_PREOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PreSetInformationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
)
{
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    if (!IsActionMonitored(Data, KmUmShared::Feature::featureMonitorFileSetInformation))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FILE_STREAM_CONTEXT* context = nullptr;
    auto status = GetSetStreamContext(Data, FltObjects, &context);
    if (!NT_SUCCESS(status))
    {
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FltReleaseContext(context);
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostSetInformationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(CompletionContext);
    FLT_POSTOP_CALLBACK_STATUS callbackStatus = FLT_POSTOP_FINISHED_PROCESSING;

    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
    {
        return callbackStatus;
    }

    FltDoCompletionProcessingWhenSafe(Data, FltObjects, CompletionContext, Flags, PostSetInformationSafeCallback, &callbackStatus);
    return callbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS FLTAPI
Minifilter::FileFilter::PostSetInformationSafeCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_opt_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
)
{
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(CompletionContext);


    auto fileInfoBuffer = (FILE_DISPOSITION_INFORMATION*)Data->Iopb->Parameters.SetFileInformation.InfoBuffer;
    auto informationClass = Data->Iopb->Parameters.SetFileInformation.FileInformationClass;

    switch (informationClass)
    {
    case FileDispositionInformation:
        if (fileInfoBuffer->DeleteFile)
        {
            NotifyEvent<KmUmShared::FileDeleteMessage>(Data, FltObjects);
        }
        break;
    default:
        NotifyEvent<KmUmShared::FileSetInformationMessage>(Data, FltObjects);
    }

    return FLT_POSTOP_FINISHED_PROCESSING;
}