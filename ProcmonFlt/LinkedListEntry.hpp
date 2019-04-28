#ifndef __LINKED_LIST_ENTRY_HPP__ 
#define __LINKED_LIST_ENTRY_HPP__

#include "cpp_allocator_object.hpp"

namespace Cpp
{
    class LinkedListEntry : public CppNonPagedObject<'TLL#'>
    {
    public:
        LinkedListEntry()
        {
            flink = this;
            blink = this;
            Validate();
        }

        virtual ~LinkedListEntry() = default;

        LinkedListEntry(const LinkedListEntry& Other) = delete;
        LinkedListEntry(LinkedListEntry&& Other) = delete;


        LinkedListEntry* flink = nullptr;
        LinkedListEntry* blink = nullptr;
    };
}

#endif //__LINKED_LIST_ENTRY_HPP__