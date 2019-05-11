#include <CppExternals.hpp>
#include <malloc.h>

namespace Cpp
{
    void* __cdecl LibAlloc(size_t Size)
    {
        return malloc(Size);
    }

    void __cdecl LibFree(void* Block)
    {
        free(Block);
    }

    void __cdecl LibCopyMemory(void* Destination, const void* Source, size_t Size)
    {
        for (size_t i = 0; i < Size; ++i)
        {
            ((unsigned __int8*)Destination)[i] = ((unsigned __int8*)Source)[i];
        }
    }

    void* __cdecl LibAllocNonpaged(size_t Size)
    {
        return malloc(Size);
    }

    void __cdecl LibFreeNonpaged(void* Block)
    {
        free(Block);
    }
}