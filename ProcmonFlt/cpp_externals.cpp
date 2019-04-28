#include <fltKernel.h>
#include <CppExternals.hpp>

#include "cpp_tags.hpp"

namespace Cpp
{
    void* __cdecl LibAlloc(size_t Size)
    {
        return ExAllocatePoolWithTag(PagedPool, Size, CPP_TAG_DEF);
    }

    void __cdecl LibFree(void* Block)
    {
        ExFreePoolWithTag(Block, CPP_TAG_DEF);
    }

    void __cdecl LibCopyMemory(void* Destination, const void* Source, size_t Size)
    {
        RtlCopyMemory(Destination, Source, Size);
    }
}