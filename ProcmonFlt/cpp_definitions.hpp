#ifndef __CPP_DEFINITIONS_HPP__ 
#define __CPP_DEFINITIONS_HPP__

#include <fltKernel.h>
#include "cpp_tags.hpp"

void* __cdecl
operator new(
    _In_ size_t Size
);

void* __cdecl
operator new[](
    _In_ size_t Size
);

void __cdecl
operator delete(
    _Inout_  PVOID Block,
    _In_ ULONG Tag
);

void __cdecl
operator delete[](
    _Inout_  PVOID Block,
    _In_ ULONG Tag
);

void __cdecl
operator delete(
    _Inout_  PVOID Block
);

void __cdecl
operator delete[](
    _Inout_  PVOID Block
);

void __cdecl
operator delete(
    _Inout_ PVOID Block,
    _In_ size_t Size
);

void __cdecl
operator delete[](
    _Inout_ PVOID Block,
    _In_ size_t Size
);

extern "C"
{
    extern int _fltused;
}

#endif //__CPP_DEFINITIONS_HPP__