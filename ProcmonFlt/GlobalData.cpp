#include "GlobalData.hpp"
#include "DriverRoutines.hpp"
#include "FileFilter.hpp"

GLOBAL_DATA gDrvData;

static UNICODE_STRING gFltPortName = RTL_CONSTANT_STRING(L"\\MyCommunicationPort");
static UNICODE_STRING gStringToBeReplaced = RTL_CONSTANT_STRING(L"very hot");
static UNICODE_STRING gStringToReplace = RTL_CONSTANT_STRING(L"very cold");

static CONST FLT_OPERATION_REGISTRATION Callbacks[] =
{
    { 
        IRP_MJ_CREATE,
        0,
        Minifilter::FileFilter::PreCreateCallback,
        Minifilter::FileFilter::PostCreateCallback,
        nullptr
    },

    {
        IRP_MJ_CLOSE,
        0,
        Minifilter::FileFilter::PreCloseCallback,
        Minifilter::FileFilter::PostCloseCallback,
        nullptr
    },

    {
        IRP_MJ_CLEANUP,
        0,
        Minifilter::FileFilter::PreCleanupCallback,
        Minifilter::FileFilter::PostCleanupCallback,
        nullptr
    },
    
    {
        IRP_MJ_WRITE,
        0,
        Minifilter::FileFilter::PreWriteCallback,
        Minifilter::FileFilter::PostWriteCallback,
        nullptr
    },

    {
        IRP_MJ_READ,
        0,
        Minifilter::FileFilter::PreReadCallback,
        Minifilter::FileFilter::PostReadCallback,
        nullptr
    },

    {
        IRP_MJ_SET_INFORMATION,
        0,
        Minifilter::FileFilter::PreSetInformationCallback,
        Minifilter::FileFilter::PostSetInformationCallback,
        nullptr
    },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_CONTEXT_REGISTRATION Contexts[] = {
    {
        FLT_STREAM_CONTEXT,
        0,
        Minifilter::FileFilter::FileContextCleanup,
        sizeof(Minifilter::FILE_STREAM_CONTEXT),
        'XTS#',
        nullptr,
        nullptr,
        nullptr
    },
    { FLT_CONTEXT_END }
};

static CONST FLT_REGISTRATION FilterRegistration =
{
    sizeof(FLT_REGISTRATION),   //  Size
    FLT_REGISTRATION_VERSION,   //  Version
    0,                          //  Flags

    Contexts,                   //  Context
    Callbacks,                  //  Operation callbacks

    DriverUnload,               //  MiniFilterUnload

    InstanceSetup,              //  InstanceSetup
    InstanceQueryTeardown,      //  InstanceQueryTeardown
    InstanceTeardownStart,      //  InstanceTeardownStart
    InstanceTeardownComplete,   //  InstanceTeardownComplete

    nullptr,                    //  GenerateFileName
    nullptr,                    //  GenerateDestinationFileName
    nullptr                     //  NormalizeNameComponent

};


void GdrvInitGlobalData(_In_ PDRIVER_OBJECT DriverObject)
{
    gDrvData.DriverObject = DriverObject;
    gDrvData.FilterHandle = nullptr;
    gDrvData.FilterRegistration = FilterRegistration;
    gDrvData.CommunicationPortName = gFltPortName;
    gDrvData.Altitude = RTL_CONSTANT_STRING(L"370030");
    gDrvData.Cookie = { 0 };

    ExInitializeRundownProtection(&gDrvData.RundownProtection);    
    gDrvData.ZwQueryInformationProcess = nullptr;

    gDrvData.NetworkStringToBeReplaced = Cpp::NonPagedString{ (unsigned __int8*)gStringToBeReplaced.Buffer, gStringToBeReplaced.Length };
    gDrvData.NetworkStringToReplace = Cpp::NonPagedString{ (unsigned __int8*)gStringToReplace.Buffer, gStringToReplace.Length };
    gDrvData.NetworkStringToReplaceMdl = nullptr;
}

void GdrvUninitGlobalData()
{
    gDrvData.ModuleFilter.Update(nullptr);
    gDrvData.ProcessFilter.Update(nullptr);
    gDrvData.ThreadFilter.Update(nullptr);
    gDrvData.RegistryFilter.Update(nullptr);
    gDrvData.NetworkFilter.Update(nullptr);

    gDrvData.CommunicationPort.Update(nullptr);
    gDrvData.CommunicationPortName = { 0,0,nullptr };

    gDrvData.ConfigurationManager.Update(nullptr);
    gDrvData.ProcessColector.Update(nullptr);
    gDrvData.BlackList.Update(nullptr);

    gDrvData.Altitude = { 0,0,nullptr };
    gDrvData.Cookie = { 0 };

    gDrvData.FilterHandle = nullptr;
    gDrvData.DriverObject = nullptr;
    gDrvData.FilterRegistration = { 0 };

    gDrvData.ZwQueryInformationProcess = nullptr;

    if (gDrvData.NetworkStringToReplaceMdl)
    {
        IoFreeMdl(gDrvData.NetworkStringToReplaceMdl);
    }
}

NTSTATUS GdrvSolveDynamicFunctions()
{
    UNICODE_STRING function = RTL_CONSTANT_STRING(L"ZwQueryInformationProcess");
    gDrvData.ZwQueryInformationProcess = (PFUNC_ZwQueryInformationProcess)MmGetSystemRoutineAddress(&function);

    if (!gDrvData.ZwQueryInformationProcess)
    {
        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

NTSTATUS GdrvAllocateStringToReplaceMdl()
{
    gDrvData.NetworkStringToReplaceMdl= IoAllocateMdl(
        gDrvData.NetworkStringToReplace.GetNakedPointer(),  // VirtualAddress
        gDrvData.NetworkStringToReplace.GetSize(),          // Length
        false,                                              // SecondaryBuffer
        false,                                              // ChargeQuota
        nullptr                                             // Irp
    );

    if (!gDrvData.NetworkStringToReplaceMdl)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MmBuildMdlForNonPagedPool(gDrvData.NetworkStringToReplaceMdl);
    return STATUS_SUCCESS;
}
