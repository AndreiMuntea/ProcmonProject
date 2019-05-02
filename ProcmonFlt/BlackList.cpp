#include "BlackList.hpp"
#include "cpp_lockguard.hpp"



Minifilter::BlackListPath::BlackListPath(UNICODE_STRING * Path)
{
    if (Path->Buffer)
    {
        path.Buffer = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool, Path->Length, 'KLB#');
    }

    if (path.Buffer)
    {
        RtlCopyMemory(path.Buffer, Path->Buffer, Path->Length);
        path.Length = Path->Length;
        path.MaximumLength = Path->MaximumLength;
    }

}

Minifilter::BlackListPath::~BlackListPath()
{
    if (path.Buffer)
    {
        ExFreePoolWithTag(path.Buffer, 'KLB#');
    }
}

Minifilter::BlackList::BlackList()
{
    Validate();
}

bool Minifilter::BlackList::IsBlackListed(UNICODE_STRING * Path)
{
    Cpp::SharedLockguard guard{ &this->lock };
    auto path = GetBlacklistedPath(Path);
    
    return (path != nullptr) ? true
                             : false;
}

Minifilter::BlackListPath * Minifilter::BlackList::GetBlacklistedPath(UNICODE_STRING * Path)
{
    for (auto it = this->blackListedPaths.begin(); it != this->blackListedPaths.end(); ++it)
    {
        if (RtlCompareUnicodeString(&it.GetRawPointer()->path, Path, TRUE) == 0)
        {
            return it.GetRawPointer();
        }
    }

    return nullptr;
}

NTSTATUS Minifilter::BlackList::Blacklist(UNICODE_STRING * Path)
{
    auto path = new BlackListPath{ Path };
    if (!path || !path->path.Buffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Cpp::ExclusiveLockguard guard{ &this->lock };
    if (GetBlacklistedPath(Path) != nullptr)
    {
        delete path;
        return STATUS_ALREADY_COMMITTED;
    }

    this->blackListedPaths.InsertTail(path);
    return STATUS_SUCCESS;
}

NTSTATUS Minifilter::BlackList::Whitelist(UNICODE_STRING * Path)
{
    Cpp::ExclusiveLockguard guard{ &this->lock };
    auto path = GetBlacklistedPath(Path);
    if (path)
    {
        this->blackListedPaths.RemoveEntry(path);
        delete path;
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_FOUND;
}
