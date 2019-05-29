#include "ScanCodeContext.hpp"
#include "GlobalData.hpp"

Cpp::ScanCodeContext::ScanCodeContext(
    _In_ PKEYBOARD_INPUT_DATA InputDataStart,
    _In_ PKEYBOARD_INPUT_DATA InputDataEnd
)
{
    USHORT noKeys = 0;
    for (PKEYBOARD_INPUT_DATA data = InputDataStart; data != InputDataEnd; data++)
    {
        ++noKeys;
    }

    auto scanCodes = (USHORT*)ExAllocatePoolWithTag(NonPagedPool, noKeys * (sizeof(USHORT) + sizeof(USHORT)), DRV_TAG_SCT);
    if (!scanCodes)
    {
        return;
    }

    this->buffer.Length = noKeys * (sizeof(USHORT) + sizeof(USHORT));
    this->buffer.MaximumLength = noKeys * (sizeof(USHORT) + sizeof(USHORT));
    this->buffer.Buffer = (PWCHAR)(scanCodes);
    
    USHORT idx = 0;
    for (PKEYBOARD_INPUT_DATA data = InputDataStart; data != InputDataEnd; data++)
    {
        this->buffer.Buffer[idx++] = data->MakeCode;
        this->buffer.Buffer[idx++] = L'\n';
    }

    Validate();
}

Cpp::ScanCodeContext::~ScanCodeContext()
{
    if (this->buffer.Buffer)
    {
        ExFreePoolWithTag(this->buffer.Buffer, DRV_TAG_SCT);
    }
}

UNICODE_STRING Cpp::ScanCodeContext::GetBuffer()
{
    return buffer;
}

void ThreadPoolLogScanCodeCallback(
    PVOID Context
)
{
    Cpp::ScanCodeContext* context = (Cpp::ScanCodeContext*)Context;
    gDrvData.File->Write(context->GetBuffer());

    delete context;
}

void ThreadPoolLogScanCodeCleanupCallback(
    PVOID Context
)
{
    Cpp::ScanCodeContext* context = (Cpp::ScanCodeContext*)Context;
    delete context;
}
