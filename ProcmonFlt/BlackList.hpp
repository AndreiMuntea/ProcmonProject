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
        BlackListPath(UNICODE_STRING* Path);
        virtual ~BlackListPath();
    private:
        UNICODE_STRING path = { 0,0,nullptr };
    };
    class BlackList : public Cpp::CppNonPagedObject<'KLB#'>
    {
    public:
        BlackList();
        virtual ~BlackList() = default;

        bool IsBlackListed(UNICODE_STRING * Path);
        NTSTATUS Blacklist(UNICODE_STRING * Path);
        NTSTATUS Whitelist(UNICODE_STRING * Path);

    private:
        BlackListPath* GetBlacklistedPath(UNICODE_STRING * Path);

        Cpp::LinkedList<BlackListPath> blackListedPaths;
        Cpp::Pushlock lock;
    };
}

#endif //__BLACK_LIST_HPP__