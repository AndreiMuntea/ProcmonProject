#include "BlackList.hpp"
#include "cpp_lockguard.hpp"

Minifilter::BlackListPath::BlackListPath(const Cpp::String& Path) :
    path{Path}
{
}

Minifilter::BlackListPath::~BlackListPath()
{
}

Minifilter::BlackList::BlackList()
{
    Validate();
}

bool Minifilter::BlackList::IsBlackListed(const Cpp::String& Path)
{
    Cpp::SharedLockguard guard{ &this->lock };
    auto path = GetBlacklistedPath(Path);
    
    return (path != nullptr) ? true
                             : false;
}

Minifilter::BlackListPath * Minifilter::BlackList::GetBlacklistedPath(const Cpp::String& Path)
{
    for (auto it = this->blackListedPaths.begin(); it != this->blackListedPaths.end(); ++it)
    {
        if (it.GetRawPointer()->path == Path)
        {
            return it.GetRawPointer();
        }
    }

    return nullptr;
}

NTSTATUS Minifilter::BlackList::Blacklist(const Cpp::String& Path)
{
    auto path = new BlackListPath{ Path };
    if (!path || !path->path.IsValid())
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

NTSTATUS Minifilter::BlackList::Whitelist(const Cpp::String& Path)
{
    Cpp::ExclusiveLockguard guard{ &this->lock };
    auto path = GetBlacklistedPath(Path);
    if (path)
    {
        delete path;
        return STATUS_SUCCESS;
    }

    return STATUS_NOT_FOUND;
}
