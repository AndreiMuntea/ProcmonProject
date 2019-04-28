#ifndef __CPP_EXTERNALS_HPP__ 
#define __CPP_EXTERNALS_HPP__

namespace Cpp
{
    void* __cdecl LibAlloc(size_t Size);
    void __cdecl LibFree(void* Block);
    void __cdecl LibCopyMemory(void* Destination, const void* Source, size_t Size);
}
#endif //__CPP_EXTERNALS_HPP__