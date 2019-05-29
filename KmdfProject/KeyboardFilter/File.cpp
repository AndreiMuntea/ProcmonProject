#include "File.hpp"

Cpp::File::File(_In_ UNICODE_STRING Path)
{
    OBJECT_ATTRIBUTES attributes = { 0 };
    IO_STATUS_BLOCK iosb = { 0 };

    InitializeObjectAttributes(&attributes, &Path, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, nullptr, nullptr);

    auto status = ZwCreateFile(
        &this->handle,
        GENERIC_ALL | SYNCHRONIZE,
        &attributes,
        &iosb,
        nullptr,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ,
        FILE_OPEN_IF,
        FILE_SYNCHRONOUS_IO_NONALERT,
        nullptr,
        0
    );

    if (NT_SUCCESS(status) && NT_SUCCESS(iosb.Status))
    {
        Validate();
    }
    else
    {
        this->handle = nullptr;
        Invalidate();
    }
}

Cpp::File::~File()
{
    if (this->handle)
    {
        ZwClose(this->handle);
        this->handle = nullptr;
    }
}

NTSTATUS 
Cpp::File::Write(_In_ UNICODE_STRING Buffer)
{
    IO_STATUS_BLOCK iosb = { 0 };

    auto status = ZwWriteFile(
        this->handle,
        nullptr,
        nullptr,
        nullptr,
        &iosb,
        Buffer.Buffer,
        Buffer.Length,
        nullptr,
        nullptr
    );

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    return iosb.Status;
}
