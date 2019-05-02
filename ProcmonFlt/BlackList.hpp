#ifndef __BLACK_LIST_HPP__ 
#define __BLACK_LIST_HPP__

#include <CppString.hpp>

#include "cpp_allocator_object.hpp"
#include "LinkedList.hpp"
#include "cpp_lock.hpp"

namespace Minifilter
{
    class BlackListPath : public Cpp::LinkedListEntry
    {
        friend class BlackList;
    public:
        BlackListPath(const Cpp::String& Path);
        virtual ~BlackListPath();
    private:
        Cpp::String path;
    };
    class BlackList : public Cpp::CppNonPagedObject<'KLB#'>
    {
    public:
        BlackList();
        virtual ~BlackList() = default;

        bool IsBlackListed(const Cpp::String& Path);
        NTSTATUS Blacklist(const Cpp::String& Path);
        NTSTATUS Whitelist(const Cpp::String& Path);

    private:
        BlackListPath* GetBlacklistedPath(const Cpp::String& Path);

        Cpp::LinkedList<BlackListPath> blackListedPaths;
        Cpp::Pushlock lock;
    };
}

#endif //__BLACK_LIST_HPP__