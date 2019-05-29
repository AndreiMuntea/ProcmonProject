#ifndef __CPP_ALLOCATOR_OBJECT_HPP__ 
#define __CPP_ALLOCATOR_OBJECT_HPP__

#include <ntddk.h>
#include <wdf.h>
#include "cpp_object.hpp"

namespace Cpp
{
    template <typename POOL_TYPE PoolType, typename ULONG Tag>
    class CppAllocatorObject : public Object
    {
    public:

        void* __cdecl operator new(_In_ size_t Size)
        {
            return ExAllocatePoolWithTag(PoolType, Size, Tag);
        }

        void* __cdecl operator new[](_In_ size_t Size)
        {
            return ExAllocatePoolWithTag(PoolType, Size, Tag);
        }

            void __cdecl operator delete(_Inout_ PVOID Block)
        {
            ExFreePoolWithTag(Block, Tag);
        }

        void __cdecl operator delete[](_Inout_ PVOID Block)
        {
            ExFreePoolWithTag(Block, Tag);
        }
    private:
    };

    template <typename ULONG Tag>
    class CppPagedObject : public CppAllocatorObject<PagedPool, Tag>
    {
    };

    template <typename ULONG Tag>
    class CppNonPagedObject : public CppAllocatorObject<NonPagedPoolNx, Tag>
    {
    };
}

#endif //__CPP_ALLOCATOR_OBJECT_HPP__